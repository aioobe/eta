#ifndef MEASUREMENTS_H
#define MEASUREMENTS_H

#include <time.h>
#include "eta.h"

typedef struct timespec timestamp_t;

typedef struct {
  timestamp_t timestamp;
  value_t value;
} Measurement;

Measurement *oldest_measurement();
Measurement *next_measurement(Measurement *m);
Measurement *last_measurement();
void add_measurement(timestamp_t timestamp, value_t value);

#endif /* MEASUREMENTS_H */
