#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#include "printing.h"
#include "options.h"

#define NOTTY_WIDTH 80

#define MINUTE (60)
#define HOUR (60 * MINUTE)
#define DAY (24 * HOUR)

// Transform actual progress to the progress we want to present to the user.
// Sounds strange? Read on...
double presented_progress(double actual) {

  // This avoids things like "-0.00%"
  double presented = actual <= 0 ? 0
                   : actual >= 1 ? 1
                   : actual;
  
  // We don't want to print 100% until it's completely done. So we translate the
  // range from 0...100% to 0...99%. Actual 100% will be handled elsewhere.
  if (presented < 1) {
    presented *= 0.99;
  }
  
  return presented;
}

// Returns terminal width or -1 if no TTY is available
int get_terminal_width() {
  struct winsize w;
  return ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) < 0 ? -1 : w.ws_col;
}

// Returns the total width to be used for the output.
unsigned int total_width(unsigned int specified_width, int terminal_width) {
  return specified_width == USE_TERMINAL_WIDTH
      ? terminal_width == -1
          ? NOTTY_WIDTH
          : terminal_width
      : specified_width;
}

// Returns the number of digits in n.
unsigned int num_digits(int n) {
  int i = 1;
  for (; n /= 10; i++);
  return i;
}

// Initialized the next output field.
void init_next_field(bool *is_first_field, bool cr_on_first) {
  if (!*is_first_field) {
    putchar(' ');
  } else if (cr_on_first) {
    putchar('\r');
  }
  *is_first_field = false;
}

// Prints progress and eta.
void print_progress_and_eta(unsigned int specified_width, value_t max_value,
                            value_t current_value, double ratio_completed,
                            double seconds_left) {

  int terminal_width = get_terminal_width();
  bool tty = terminal_width != -1;
  unsigned int width = total_width(specified_width, terminal_width);

  // Default widths
  unsigned int w_current_value = num_digits(max_value);
  unsigned int w_percent = 4; // Ex: "100%"
  unsigned int w_eta = 14;    // Ex: "ETA 12h 16m 5s"

  // Min width. Compute final width later.
  unsigned int w_progress_bar = 7; // "[###  ]"

  // 4 blocks -> 3 separating spaces
  int separators = 3;

  if (width < w_current_value + w_progress_bar + w_percent + w_eta+separators) {
    w_current_value = 0;
    separators--;
  }

  if (width < w_progress_bar + w_percent + w_eta + separators) {
    w_progress_bar = 0;
    separators--;
  }

  if (width < w_percent + w_eta) {
    w_eta = 0;
    separators--;
  }

  if (width < w_percent) {
    // aww, come on...
    width = w_percent;
  }

  // Let progress bar grow
  if (w_progress_bar) {
    w_progress_bar = width - w_current_value - w_percent - w_eta - separators;
  }

  bool first_field = true;

  // Current value
  if (w_current_value) {
    init_next_field(&first_field, tty);
    printf("%*llu", w_current_value, current_value);
  }
  
  // Progress bar
  if (w_progress_bar) {
    init_next_field(&first_field, tty);
    putchar('[');
    int filled = (int) (ratio_completed * (w_progress_bar - 2));
    for (int i = 0; i < w_progress_bar - 2; i++) {
      putchar(i < filled ? '#' : ' ');
    }
    putchar(']');
  }

  // Percent
  if (w_percent) {
    init_next_field(&first_field, tty);
    double percent = 100.0 * presented_progress(ratio_completed);
    printf("%*.0f%%", w_percent - 1, percent);
  }
  
  // ETA
  if (w_eta) {
    init_next_field(&first_field, tty);
    int w_eta_left = w_eta;
    if (seconds_left <= 0) {
      char *str = seconds_left == 0 ? "Done" : "ETA n/a";
      if (w_eta_left >= strlen(str)) {
        printf("%s", str);
        w_eta_left -= strlen(str);
      }
    } else {
      int seconds = (int) (seconds_left + .5);
      int days = seconds / DAY;
      seconds %= DAY;
      int hours = seconds / HOUR;
      seconds %= HOUR;
      int minutes = seconds / MINUTE;
      seconds %= MINUTE;
      
      if (w_eta_left >= 3) {
        printf("%s", "ETA");
        w_eta_left -= 3;
      }
      
      int fields[] = { days, hours, minutes, seconds };
      char units[] = { 'd', 'h', 'm', 's' };
      int n_fields = sizeof(units);
      bool no_field_printed = true;
      for (int i = 0; i < n_fields; i++) {
        bool last_chance = i == n_fields - 1;
        if (fields[i] || (last_chance && no_field_printed)) {

          int digits = num_digits(fields[i]);
          int actual_width = digits + 2; // +2: ' ' separator and unit char

          // The last field can fluctuate between, say 9 and 10, which means
          // that it may sometimes fit, sometimes not (which causes it to
          // "flicker"). To avoid this we require it to fit even if it's 2
          // digits.
          int required_width = MAX(2, digits) + 2;
          if (w_eta_left < required_width) {
            break;
          }

          printf(" %d%c", fields[i], units[i]);
          no_field_printed = false;
          w_eta_left -= actual_width;
        }
      }
    }

    if (tty) {

      static int prev_w_eta = 0;

      int current_w_eta = w_eta - w_eta_left;
      int to_cover = prev_w_eta - current_w_eta;

      // Since we're using '\r', make sure we fill the field completely so we
      // cover the previous output.
      while (to_cover-- > 0) {
        putchar(' ');
      }

      prev_w_eta = w_eta - w_eta_left;
    }
  }

  if (!tty || seconds_left == 0) {
    putchar('\n');
  }

  fflush(stdout);
}
