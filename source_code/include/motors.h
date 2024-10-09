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

#define MOTOR_MAX_RPM 900

#define MOTOR_SPEED_KP 8.0
#define MOTOR_SPEED_KI 0.1
#define MOTOR_SPEED_KD 0.0

void motors_init(void);
void motors_disable(void);
void motors_enable(void);
void motors_move(void);

void set_motor_left_inited(void);
void set_motor_right_inited(void);

int16_t motor_left_get_speed_factor(void);
int16_t motor_right_get_speed_factor(void);

void motors_set_left_speed(int left_speed);
void motors_set_right_speed(int right_speed);

void debug_motors_move_open_loop(void);

#endif