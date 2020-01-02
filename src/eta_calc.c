#include <stdbool.h>
#include "eta_calc.h"

#include <stdio.h>

// Compute and return ratio completed
double compute_ratio_completed(value_t initial_value, value_t target_value) {
  value_t value = last_measurement()->value;
  return (double) (value - initial_value) / (target_value - initial_value);
}

// Calculates the rate of change from m1 to m2.
double rate(Measurement *m1, Measurement *m2) {
  value_t progress_delta = m2->value - m1->value;
  long sec_delta = m2->timestamp.tv_sec - m1->timestamp.tv_sec;
  long nsec_delta = m2->timestamp.tv_nsec - m1->timestamp.tv_nsec;
  double time_delta = sec_delta + nsec_delta / 1.0e9;
  return progress_delta / time_delta;
}

// Calculate the average rate of change.
//
// This function assumes that we have at least two measurements.
double average_rate() {
  Measurement *oldest = oldest_measurement();
  Measurement *latest = last_measurement();
  return rate(oldest, latest);
}

// Calculate and return estimated seconds left.
double compute_seconds_left(value_t target_value, bool down) {

  Measurement *m = oldest_measurement();

  // 0 measurements available?
  if (m == NULL) {
    // Can't determine rate -> can't determine seconds left.
    return -1;
  }
  
  // Target value reached?
  if (down ? m->value <= target_value : m->value >= target_value) {
    return 0;
  }
  
  // Only 1 measurement available?
  if (m == last_measurement()) {
    // Can't determine rate -> can't determine seconds left.
    return -1;
  }

  // Note: work_left may be negative, but then so will average_rate be.
  value_t work_left = target_value - last_measurement()->value;
  return work_left / average_rate();
}
