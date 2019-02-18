#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdbool.h>
#include "eta.h"

#define START_MODE_INITIAL -1
#define USE_TERMINAL_WIDTH -1

typedef struct {
  value_t target_value;
  value_t start_mode;        // Value to be treated as start value (-1 = first measurement)
  unsigned int interval;  // Seconds to sleep between invocations of external command
  bool down;
  bool cont;
  unsigned int output_width;
  int n_cmd;
  char **cmd;
} Options;

void parse_options(int argc, char** argv, Options *options);

#endif /* OPTIONS_H */
