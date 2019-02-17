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
// This function assumes that we have at least tmo measurements.
double average_rate() {
  
  Measurement *oldest = oldest_measurement();
  Measurement *latest = last_measurement();

  return rate(oldest, latest);

  // The problem with the calculation below is that when using --cont we may
  // have a situation like this:
  //
  //   0 <500ms> 1 <500ms> 2 <1ms> 3 <500ms> 4 <500ms> ...
  //
  // The average rate of change for the 2 -> 3 delta will get huge and ruin the
  // result completely.

  /*
  // 1*rate[-k] + 2*rate[-k+1] + ... + k*rate[0]
  // -------------------------------------------
  //           1 + 2 + 3 + ... + k

  double numerator = 0;
  double denominator = 0;
  int weight = 1;
  Measurement *m = oldest_measurement();
  do {
    Measurement *m_prev = m;
    m = next_measurement(m);
    numerator += weight * rate(m_prev, m);
    denominator += weight;
    weight++;
  } while (m != last_measurement());

  return numerator / denominator;
  */
}

// Calculate and return estimated seconds left.
double compute_seconds_left(value_t target_value, bool down) {

  Measurement *m = oldest_measurement();

  // 0 available measurements?
  if (m == NULL) {
    // Can't determine rate -> can't determine seconds left.
    return -1;
  }
  
  // 1 available measurement?
  if (m == last_measurement()) {
    // Can only determine seconds left if already 100%
    return m->value >= target_value ? 0 : -1;
  }

  value_t work_left = target_value - last_measurement()->value;

  if ((down ? -work_left : work_left) <= 0) {
    return 0;
  }
  
  return work_left / average_rate();
}
