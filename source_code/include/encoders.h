#ifndef __ENCODERS_H
#define __ENCODERS_H

#include <stdint.h>

#include <libopencm3/stm32/timer.h>

#include "config.h"
#include "setup.h"

int32_t get_encoder_left_total_ticks(void);
int32_t get_encoder_right_total_ticks(void);

int32_t max_likelihood_counter_diff(uint16_t now, uint16_t before);
void update_encoder_readings(void);

#endif
