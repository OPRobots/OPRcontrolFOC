#ifndef __MOTORS_H
#define __MOTORS_H

#include <stdint.h>
#include <math.h>
#include <fastmath.h>

#include "setup.h"
#include "config.h"

#define SINE_LOOKUP_STEP 5
#define SINE_LOOKUP_SIZE 300

void motors_init(void);
void motors_move(void);

#endif