#include "motors.h"

static bool inited = false;
static uint16_t sine_lookup[SINE_LOOKUP_SIZE];

static uint16_t motor_left_init_index = 0;
static volatile uint16_t motor_left_A;
static volatile uint16_t motor_left_B;
static volatile uint16_t motor_left_C;

static uint16_t motor_right_init_index = 0;
static volatile uint16_t motor_right_A;
static volatile uint16_t motor_right_B;
static volatile uint16_t motor_right_C;

static void fill_lookup(void) {
  float value = 0;
  for (uint16_t i = 0; i < SINE_LOOKUP_SIZE; i++) {
    value = sin((i / (SINE_LOOKUP_SIZE * 1.0f)) * 2 * M_PI);
    value *= PWM_PERIOD / 2.0f;
    value += PWM_PERIOD / 2.0f;
    value = abs(round(value));
    sine_lookup[i] = (uint16_t)value;
  }
}

static void set_starting_index(void) {
  motor_left_A = motor_left_init_index;
  motor_left_B = motor_left_A + (SINE_LOOKUP_SIZE / 3);
  if (motor_left_B >= SINE_LOOKUP_SIZE) {
    motor_left_B = motor_left_B - SINE_LOOKUP_SIZE;
  }
  motor_left_C = motor_left_B + (SINE_LOOKUP_SIZE / 3);
  if (motor_left_C >= SINE_LOOKUP_SIZE) {
    motor_left_C = motor_left_C - SINE_LOOKUP_SIZE;
  }

  motor_right_A = motor_right_init_index;
  motor_right_B = motor_right_A + (SINE_LOOKUP_SIZE / 3);
  if (motor_right_B >= SINE_LOOKUP_SIZE) {
    motor_right_B = motor_right_B - SINE_LOOKUP_SIZE;
  }
  motor_right_C = motor_right_B + (SINE_LOOKUP_SIZE / 3);
  if (motor_right_C >= SINE_LOOKUP_SIZE) {
    motor_right_C = motor_right_C - SINE_LOOKUP_SIZE;
  }
}

void motors_init(void) {
  fill_lookup();
  set_starting_index();
  inited = true;
}

void motors_move(void) {
  if (inited) {
    motor_left_A += SINE_LOOKUP_STEP;
    motor_left_B += SINE_LOOKUP_STEP;
    motor_left_C += SINE_LOOKUP_STEP;
    if (motor_left_A >= SINE_LOOKUP_SIZE) {
      motor_left_A = motor_left_A - SINE_LOOKUP_SIZE;
    }
    if (motor_left_B >= SINE_LOOKUP_SIZE) {
      motor_left_B = motor_left_B - SINE_LOOKUP_SIZE;
    }
    if (motor_left_C >= SINE_LOOKUP_SIZE) {
      motor_left_C = motor_left_C - SINE_LOOKUP_SIZE;
    }
    // printf("%d %d %d\n", sine_lookup[motor_left_A], sine_lookup[motor_left_B], sine_lookup[motor_left_C]);

    motor_right_A += SINE_LOOKUP_STEP;
    motor_right_B += SINE_LOOKUP_STEP;
    motor_right_C += SINE_LOOKUP_STEP;
    if (motor_right_A >= SINE_LOOKUP_SIZE) {
      motor_right_A = motor_right_A - SINE_LOOKUP_SIZE;
    }
    if (motor_right_B >= SINE_LOOKUP_SIZE) {
      motor_right_B = motor_right_B - SINE_LOOKUP_SIZE;
    }
    if (motor_right_C >= SINE_LOOKUP_SIZE) {
      motor_right_C = motor_right_C - SINE_LOOKUP_SIZE;
    }

    // timer_set_oc_value(TIM1, TIM_OC1, sine_lookup[motor_left_A]);
    TIM_CCR1(TIM1) = sine_lookup[motor_left_A];
    // timer_set_oc_value(TIM1, TIM_OC2, sine_lookup[motor_left_B]);
    TIM_CCR2(TIM1) = sine_lookup[motor_left_B];
    // timer_set_oc_value(TIM1, TIM_OC3, sine_lookup[motor_left_C]);
    TIM_CCR3(TIM1) = sine_lookup[motor_left_C];

    // timer_set_oc_value(TIM2, TIM_OC1, sine_lookup[motor_right_A]);
    TIM_CCR1(TIM2) = sine_lookup[motor_right_A];
    // timer_set_oc_value(TIM2, TIM_OC2, sine_lookup[motor_right_B]);
    TIM_CCR2(TIM2) = sine_lookup[motor_right_B];
    // timer_set_oc_value(TIM2, TIM_OC3, sine_lookup[motor_right_C]);
    TIM_CCR3(TIM2) = sine_lookup[motor_right_C];
  }
}
