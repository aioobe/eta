#include <stdbool.h>
#include "eta_calc.h"

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

// Calculate the weighted average rate of change, where the weights decrease
// linearly the older they are.
//
// This function assumes that we have at least tmo measurements.
double average_rate() {

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
