#include "motors.h"

static bool inited = false;
static uint16_t sine_lookup[SINE_LOOKUP_SIZE];

static uint16_t motor_left_init_index = 0;
static uint16_t motor_left_offset = 0;
static volatile int motor_left_speed = 0;
static int16_t motor_left_speed_factor = 0;
static int16_t motor_left_last_error = 0;
static int16_t motor_left_sum_error = 0;
static bool motor_left_inited = false;
static volatile int16_t motor_left_A;
static volatile int16_t motor_left_B;
static volatile int16_t motor_left_C;

static uint16_t motor_right_init_index = 0;
static uint16_t motor_right_offset = 0;
static volatile int motor_right_speed = 0;
static int16_t motor_right_speed_factor = 0;
static int16_t motor_right_last_error = 0;
static int16_t motor_right_sum_error = 0;
static int motor_right_inited = false;
static volatile int16_t motor_right_A;
static volatile int16_t motor_right_B;
static volatile int16_t motor_right_C;

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

static void motors_pid(void) {
  int16_t left_error = abs(motor_left_speed) - constrain(map(abs(get_encoder_left_speed()), 0, MOTOR_MAX_RPM, 0, 100), 0, 100);
  motor_left_speed_factor =
      (left_error * MOTOR_SPEED_KP) +
      // ((left_error - motor_left_last_error) * MOTOR_SPEED_KD) +
      (motor_left_sum_error * MOTOR_SPEED_KI);

  motor_left_speed_factor = constrain(motor_left_speed_factor, 0, 100);
  if (motor_left_speed_factor > 95) {
    gpio_set(GPIOC, GPIO14);
  } else {
    gpio_clear(GPIOC, GPIO14);
  }

  if (motor_left_speed_factor < 100 || left_error < 0) {
    motor_left_sum_error += left_error;
    if (motor_left_sum_error < 0) {
      motor_left_sum_error = 0;
    }
  }
  motor_left_last_error = left_error;

  int16_t right_error = abs(motor_right_speed) - constrain(map(abs(get_encoder_right_speed()), 0, MOTOR_MAX_RPM, 0, 100), 0, 100);
  motor_right_speed_factor =
      (right_error * MOTOR_SPEED_KP) +
      // ((right_error - motor_right_last_error) * MOTOR_SPEED_KD) +
      (motor_right_sum_error * MOTOR_SPEED_KI);

  motor_right_speed_factor = constrain(motor_right_speed_factor, 0, 100);
  if (motor_right_speed_factor > 95) {
    gpio_set(GPIOC, GPIO13);
  } else {
    gpio_clear(GPIOC, GPIO13);
  }

  if (motor_right_speed_factor < 100 || right_error < 0) {
    motor_right_sum_error += right_error;
    if (motor_right_sum_error < 0) {
      motor_right_sum_error = 0;
    }
  }
  motor_right_last_error = right_error;
}

void motors_init(void) {
  if (!inited) {
    fill_lookup();
    motors_disable();
    set_starting_index();
    inited = true;
  }
}

void set_motor_left_inited(void) {
  motor_left_inited = true;
}

void set_motor_right_inited(void) {
  motor_right_inited = true;
}

void motors_disable(void) {
  gpio_clear(GPIOB, GPIO14 | GPIO15);
}

void motors_enable(void) {
  // fill_lookup();
  gpio_set(GPIOB, GPIO14 | GPIO15);
}

void motors_set_left_speed(int left_speed) {
  motor_left_speed = left_speed;
}

void motors_set_right_speed(int right_speed) {
  motor_right_speed = right_speed;
}

int16_t motor_left_get_speed_factor(void) {
  return motor_left_speed_factor;
}

int16_t motor_right_get_speed_factor(void) {
  return motor_right_speed_factor;
}

void motors_move(void) {
  if (inited) {
    motors_pid();
    if (motor_left_speed != 0) {
      if (motor_left_speed > 0) {
        motor_left_A = map(get_encoder_left_absolute_position(), 0, MAX_ABSOLUTE_POSITION, 0, 360) + 270;
      } else {
        motor_left_A = map(get_encoder_left_absolute_position(), 0, MAX_ABSOLUTE_POSITION, 0, 360) + 90;
      }

      motor_left_A = motor_left_A % 360;
      motor_left_B = (motor_left_A + (SINE_LOOKUP_SIZE / 3)) % 360;
      motor_left_C = (motor_left_B + (SINE_LOOKUP_SIZE / 3)) % 360;

      TIM_CCR1(TIM2) = sine_lookup[motor_left_C] * (abs(motor_left_speed_factor) / 100.0f);
      TIM_CCR2(TIM2) = sine_lookup[motor_left_B] * (abs(motor_left_speed_factor) / 100.0f);
      TIM_CCR3(TIM2) = sine_lookup[motor_left_A] * (abs(motor_left_speed_factor) / 100.0f);

      // printf("%d %d %d\n", sine_lookup[motor_left_A], sine_lookup[motor_left_B], sine_lookup[motor_left_C]);
      // printf("%d - %d %d %d\n",(int16_t)map(get_encoder_left_absolute_position(), 0, MAX_ABSOLUTE_POSITION, 0, 360), motor_left_A, motor_left_B, motor_left_C);
    } else {
      TIM_CCR1(TIM2) = sine_lookup[motor_left_C];
      TIM_CCR2(TIM2) = sine_lookup[motor_left_B];
      TIM_CCR3(TIM2) = sine_lookup[motor_left_A];
    }

    if (motor_right_speed != 0) {
      if (motor_right_speed > 0) {
        motor_right_A = map(get_encoder_right_absolute_position(), 0, MAX_ABSOLUTE_POSITION, 0, 360) + 90;
      } else {
        motor_right_A = map(get_encoder_right_absolute_position(), 0, MAX_ABSOLUTE_POSITION, 0, 360) + 270;
      }

      motor_right_A = motor_right_A % 360;
      motor_right_B = (motor_right_A + (SINE_LOOKUP_SIZE / 3)) % 360;
      motor_right_C = (motor_right_B + (SINE_LOOKUP_SIZE / 3)) % 360;

      TIM_CCR1(TIM1) = sine_lookup[motor_right_A] * (abs(motor_right_speed_factor) / 100.0f);
      TIM_CCR2(TIM1) = sine_lookup[motor_right_C] * (abs(motor_right_speed_factor) / 100.0f);
      TIM_CCR3(TIM1) = sine_lookup[motor_right_B] * (abs(motor_right_speed_factor) / 100.0f);

      // printf("%d %d %d\n", sine_lookup[motor_right_A], sine_lookup[motor_right_B], sine_lookup[motor_right_C]);
    } else {
      TIM_CCR1(TIM1) = sine_lookup[motor_right_A];
      TIM_CCR2(TIM1) = sine_lookup[motor_right_C];
      TIM_CCR3(TIM1) = sine_lookup[motor_right_B];
    }
  }
}

void debug_motors_move_open_loop() {
  TIM_CCR1(TIM2) = (uint16_t)(sine_lookup[motor_left_A]);
  TIM_CCR2(TIM2) = (uint16_t)(sine_lookup[motor_left_B]);
  TIM_CCR3(TIM2) = (uint16_t)(sine_lookup[motor_left_C]);

  // printf("%d %d %d\n", sine_lookup[motor_left_A], sine_lookup[motor_left_B], sine_lookup[motor_left_C]);

  motor_left_A += 1;
  if (motor_left_A >= SINE_LOOKUP_SIZE) {
    motor_left_A = motor_left_A - SINE_LOOKUP_SIZE;
  }
  motor_left_B = motor_left_A + (SINE_LOOKUP_SIZE / 3);
  if (motor_left_B >= SINE_LOOKUP_SIZE) {
    motor_left_B = motor_left_B - SINE_LOOKUP_SIZE;
  }
  motor_left_C = motor_left_B + (SINE_LOOKUP_SIZE / 3);
  if (motor_left_C >= SINE_LOOKUP_SIZE) {
    motor_left_C = motor_left_C - SINE_LOOKUP_SIZE;
  }

  TIM_CCR1(TIM1) = (uint16_t)(sine_lookup[motor_right_A]);
  TIM_CCR2(TIM1) = (uint16_t)(sine_lookup[motor_right_B]);
  TIM_CCR3(TIM1) = (uint16_t)(sine_lookup[motor_right_C]);

  motor_right_A += 1;
  if (motor_right_A >= SINE_LOOKUP_SIZE) {
    motor_right_A = motor_right_A - SINE_LOOKUP_SIZE;
  }
  motor_right_B = motor_right_A + (SINE_LOOKUP_SIZE / 3);
  if (motor_right_B >= SINE_LOOKUP_SIZE) {
    motor_right_B = motor_right_B - SINE_LOOKUP_SIZE;
  }
  motor_right_C = motor_right_B + (SINE_LOOKUP_SIZE / 3);
  if (motor_right_C >= SINE_LOOKUP_SIZE) {
    motor_right_C = motor_right_C - SINE_LOOKUP_SIZE;
  }

  delay(1);
}
