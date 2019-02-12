#ifndef PRINTING_H
#define PRINTING_H

#include "eta.h"

void print_progress_and_eta(unsigned int specified_width, value_t max_value,
                            value_t current_value, double ratio_completed,
                            double seconds_left);

#endif /* PRINTING_H */
