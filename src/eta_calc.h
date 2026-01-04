#ifndef ETA_CALC_H
#define ETA_CALC_H

#include "measurements.h"

double compute_ratio_completed(value_t initial_value, value_t target_value);
double rate(Measurement *m1, Measurement *m2);
double average_rate();
double compute_seconds_left(value_t target_value, bool down);

#endif /* ETA_CALC_H */
