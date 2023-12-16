#ifndef __MOTORS_H
#define __MOTORS_H

#include <fastmath.h>
#include <math.h>
#include <stdint.h>

#include "config.h"
#include "delay.h"
#include "encoders.h"
#include "setup.h"
#include "usart.h"
#include "utils.h"

#define SINE_LOOKUP_SIZE 360


void motors_init(void);
void motors_disable(void);
void motors_enable(void);
void motors_move(void);

void set_motor_left_inited(void);
void set_motor_right_inited(void);

void motors_set_left_speed(int left_speed);
void motors_set_right_speed(int right_speed);


#endif