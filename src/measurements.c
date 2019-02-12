#include "measurements.h"

#define MEASUREMENTS_HISTORY 10

// Cyclic buffer of measurements
static Measurement measurements[MEASUREMENTS_HISTORY];

// Measurements added so far
static int num_measurements = 0;

// Return the i:th measurement starting from 0.
// Wrapps if i > MEASUREMENTS_HISTORY
Measurement *measurement(int i) {
  return measurements + (i % MEASUREMENTS_HISTORY);
}

// Return the oldest available measurement.
Measurement *oldest_measurement() {
  if (num_measurements == 0) {
    return NULL;
  }
  int i = num_measurements - MEASUREMENTS_HISTORY;
  if (i < 0) {
    i = 0;
  }
  return measurement(i);
}

// Return the pointer the measurement after m.
Measurement *next_measurement(Measurement *m) {
  Measurement *next = m + 1;
  if (next >= measurements + MEASUREMENTS_HISTORY) {
    next = measurements;
  }
  return next;
}

// Return a pointer to the the measurement added last
Measurement *last_measurement() {
  return num_measurements == 0 ? NULL : measurement(num_measurements - 1);
}

// Adds a measurement to the cyclic buffer of mearusements
void add_measurement(timestamp_t timestamp, value_t value) {

  Measurement *last = last_measurement();

  // If we "blindly" append this value, we may (if progress is slow) end up with
  //
  //     0 0 0 0 0
  //     0 0 0 0 1
  //     0 0 0 1 1
  //     ...
  //     1 1 1 1 1
  //
  // During 0...0 and 1...1 periods, eta calculations would break.
  //
  // So instead we check if this measurement has the same value as the last one
  // and then we update the timestamp of the last measurement instead. This
  // give a situation like
  //
  //     0 1 2 3 4
  //     1 2 3 4 5
  //     ...
  //
  // The newer measurement can be seen as "strictly more interesting" than the
  // last since it came in at a later point in time.
  if (last != NULL && value == last->value) {
    last->timestamp = timestamp;
    return;
  }

  Measurement *m = last == NULL ? measurement(0) : next_measurement(last);
  m->timestamp = timestamp;
  m->value = value;
  num_measurements++;
}
