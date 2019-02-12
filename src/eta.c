#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

#include "eta.h"
#include "options.h"
#include "eta_calc.h"
#include "measurements.h"
#include "printing.h"

// Global variable to store program name (argv[0]). Used to print error messages
// on the format "./eta: some error" in various places in the code.
char *program_name;

value_t query_progress(char *cmd, bool cont);
timestamp_t now();

int main(int argc, char **argv) {
  program_name = argv[0];

  Options options;
  parse_options(argc, argv, &options);

  // Initialize later
  value_t initial_value;
  
  while (1) {

    value_t current_value = query_progress(options.progress_cmd, options.cont);

    // First measurement?
    if (oldest_measurement() == NULL) {
      initial_value = options.start_mode == START_MODE_INITIAL
          ? current_value
          : options.start_mode;
    }

    add_measurement(now(), current_value);

    double ratio_completed = compute_ratio_completed(
        initial_value, options.target_value, options.down);

    double seconds_left = compute_seconds_left(
        options.target_value, options.down);

    print_progress_and_eta(
        options.output_width,
        MAX(initial_value, options.target_value),
        current_value,
        ratio_completed,
        seconds_left);

    if (ratio_completed >= 1.0) {
      break;
    }

    if (!options.cont) {
      sleep(options.interval);
    }
  }

  free(options.progress_cmd);
  return SUCCESS;
}

// Execute cmd. Look for a numeric value in the output.
value_t query_progress(char *cmd, bool cont) {

  // !cont: Opened / closed each invocation
  //  cont: Opened / closed each invocation
  static FILE *cmd_fp = NULL;

  // Execute command if it's not already running
  if (!cmd_fp) {
    cmd_fp = popen(cmd, "r");
    if (cmd_fp == NULL) {
      printf("failed to execute external command: '%s'\n", cmd);
      exit(COMMAND_EXECUTION_FAILED);
    }
  }
  
  // Read a line of output
  char output[1000];
  if (fgets(output, sizeof(output), cmd_fp) == NULL) {
    // No output could be read. Setting the result to the empty string will
    // cause a failure when parsing later, and print an error message and exit.
    output[0] = 0;
  }

  // If no newline was read (and we're running with --cont), discard the
  // remaining line so we don't erroneously read that next time around.
  if (strchr(output, '\n') == NULL && cont) {
    for (;;) {
      int c = fgetc(cmd_fp);
      if (c == '\n' || c == EOF) {
        break;
      }
    }
  }

  // Look for first digit
  char *first_digit = output;
  while (!isdigit(*first_digit)) {
    if (*first_digit == 0) {
      printf("Could not find number in command output -- '%s'\n", output);
      exit(COULD_NOT_PARSE_COMMAND_OUTPUT);
    }
    first_digit++;
  }
  
  value_t value = strtoul(first_digit, NULL, 10);

  // Close cmd if we're not running in continuous mode.
  if (!cont) {
    int exitStatus = WEXITSTATUS(pclose(cmd_fp));
    cmd_fp = NULL;
    if (exitStatus != 0) {
      puts("External command terminated with non-zero exit code.");
      exit(COMMAND_EXECUTION_FAILED);
    }
  }
  
  return value;
}

// Return the current time
timestamp_t now() {
  timestamp_t ts;
  clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
  return ts;
}

