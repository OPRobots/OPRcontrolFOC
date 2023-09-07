#ifndef __MOTORS_H
#define __MOTORS_H

#include <stdint.h>
#include <math.h>
#include <fastmath.h>

#include "setup.h"
#include "config.h"
#include "delay.h"
#include "utils.h"
#include "usart.h"

#define SINE_LOOKUP_STEP 5
#define SINE_LOOKUP_SIZE 300

void motors_init(void);
void motors_move(void);

void motors_set_left_speed(int left_speed);
void motors_set_right_speed(int right_speed);

#endif