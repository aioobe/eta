#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>

#include "options.h"

bool isopt(char *arg, char abbr, char *full);
char *get_opt_arg(int argi, int argc, char **argv, char *opt_name);
value_t parse_suffixed_value(char *arg_name, char *arg);
int parse_numeric_arg(char *arg_name, char *arg);
void error(const char *format, ...);
void print_usage();

// Parse argv and store the result in options.
void parse_options(int argc, char **argv, Options *options) {

  // Set defaults
  options->interval = 1;
  options->start_mode = 0;
  options->output_width = USE_TERMINAL_WIDTH;
  options->down = false;
  options->cont = false;

  bool start_provided = false;
  bool interval_provided = false;

  int argi = 1;
  while (argi < argc) {
    char *arg = argv[argi++];

    if (isopt(arg, 's', "start")) {
      char *start_arg = get_opt_arg(argi++, argc, argv, "start");
      start_provided = true;
      options->start_mode = strcmp(start_arg, "initial") == 0
          ? START_MODE_INITIAL
          : parse_suffixed_value("start", start_arg);
      continue;
    }

    if (isopt(arg, 'c', "cont")) {
      options->cont = true;
      continue;
    }

    if (isopt(arg, 'd', "down")) {
      options->down = true;
      continue;
    }

    if (isopt(arg, 'i', "interval")) {
      char *interval_arg = get_opt_arg(argi++, argc, argv, "interval");
      interval_provided = true;
      options->interval = parse_numeric_arg("interval", interval_arg);
      continue;
    }

    if (isopt(arg, 'w', "width")) {
      char *width_arg = get_opt_arg(argi++, argc, argv, "width");
      options->output_width = parse_numeric_arg("width", width_arg);
      continue;
    }

    if (isopt(arg, 'h', "help")) {
      print_usage();
      exit(SUCCESS);
    }

    // Option parsing failed. We're probably looking at the target value
    // argument. Take a step back, break out of option parsing loop, and parse
    // target value next.
    argi--;
    break;
  }
  
  // Parse target value
  char *target_arg = get_opt_arg(argi++, argc, argv, "target");
  options->target_value = parse_suffixed_value("target", target_arg);

  // Parse external command
  if (argi >= argc) {
    error("no progress command provided");
  }
  options->n_cmd = argc - argi;
  options->cmd = argv + argi;
  
  // --cont and --interval are mutually exclusive
  if (interval_provided && options->cont) {
    error("at most one of --interval and --cont may be specified");
  }

  // If counting downwards, the target is often 0, so 'initial' makes more sense
  // than 0 as default for start_mode.
  if (options->down && !start_provided) {
    options->start_mode = START_MODE_INITIAL;
  }
}

// Return true if arg is the matches "-[abbr]" or "--[full]"
bool isopt(char *arg, char abbr, char *full) {
  return (strlen(arg) == 2
          && arg[0] == '-'
          && arg[1] == abbr)
      || (strlen(arg) == 2 + strlen(full)
          && arg[0] == '-'
          && arg[1] == '-'
          && strcmp(arg + 2, full) == 0);
}

// Return the next option if there's one available, otherwise print an error.
char *get_opt_arg(int argi, int argc, char **argv, char *opt_name) {
  if (argi >= argc) {
    char *fmt = strcmp(opt_name, "target") == 0
        ? "no %s argument provided"
        : "--%s requires an argument";
    error(fmt, opt_name);
  }
  return argv[argi];
}

// Translates "k" to 1000, "m" to 1000000 etc.
//
// If the argument is an empty string or a string that starts with a 1 is
// returned as this is an indication that the preceeding number does not
// have a suffix.
long long magnitude_for_suffix(char *suffix) {
  if (*suffix == '\0' || isspace(*suffix)) {
    return 1;
  }
  char *suffixes[] = {
      "k",  "m",  "g", "t",
      "ki", "mi", "gi", "ti"
  };
  long long magnitudes[] = {
      1e3, 1e6, 1e9, 1e12,
      1L << 10, 1L << 20, 1L << 30, 1L << 40
  };
  // Lower case the given suffix
  for (char *p = suffix; *p; ++p) {
    *p = tolower(*p);
  }

  for (int i = 0; i < sizeof(magnitudes)/sizeof(magnitudes[0]); i++) {
    if (strcmp(suffix, suffixes[i]) == 0) {
      return magnitudes[i];
    }
  }
  return -1;
}

// Parse a value like "15g" or "123".
// arg_name is needed only for printing an error message.
value_t parse_suffixed_value(char *arg_name, char *arg) {
  char *tail;
  value_t value = strtoul(arg, &tail, 10);
  if (arg == tail) {
    error("could not parse %s value -- '%s'", arg_name, arg);
  }
  long long magnitude = magnitude_for_suffix(tail);
  if (magnitude == -1) {
    error("invalid %s value suffix -- '%s'", tail);
  }
  return magnitude * value;
}

// Parse an integer, or print an error message if not possible.
int parse_numeric_arg(char *arg_name, char *arg) {
  char *err_fmt = "invalid %s argument -- '%s'";
  int len = strlen(arg);
  if (len == 0 || len >= 9) {
    error(err_fmt, arg_name, arg);
  }
  char *tmp = arg;
  while (*tmp) {
    if (!isdigit(*tmp++)) {
      error(err_fmt, arg_name, arg);
    }
  }
  int parsed = strtoul(arg, NULL, 10);
  if (parsed <= 0) {
    error(err_fmt, arg_name, arg);
  }
  return parsed;
}

// Print an error message and exit.
void error(const char *format, ...) {
  va_list args;
  fprintf(stderr, "%s: ", program_name);
  va_start (args, format);
  vfprintf(stderr, format, args);
  fputc('\n', stderr);
  va_end (args);
  exit(INVALID_COMMAND_LINE_ARGUMENTS);
}

// Print usage message.
void print_usage() {
  puts("Usage:");
  puts(" eta [OPTIONS] TARGET COMMAND");
  puts("");
  puts("Runs COMMAND repeatedly. For each invocation it looks for a number");
  puts("in the first line of the output, and prints an estimate for when");
  puts("the number will reach the TARGET value.");
  puts("");
  puts("  -s, --start <VALUE>|initial");
  puts("                         let VALUE (or initial value from COMMAND)");
  puts("                         represent 0%");
  puts("  -i, --interval <SECS>  run COMMAND every SECS seconds");
  puts("  -d, --down             value decreases; --start will default to");
  puts("                         'initial'");

  puts("  -w, --width <COLS>     specify width of output");
  puts("  -c, --cont             instead of running COMMAND repeatedly,");
  puts("                         keep COMMAND running and read progress");
  puts("                         continuously line by line");
  puts("  -h, --help             print this help and exit");
}
