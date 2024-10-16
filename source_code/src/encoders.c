#include "encoders.h"

/* Difference between the current count and the latest count */
static volatile int32_t left_diff_ticks;
static volatile int32_t right_diff_ticks;

/* Total number of counts */
static volatile int32_t left_total_ticks;
static volatile int32_t right_total_ticks;

/* Speed, in rpm */
static volatile int16_t left_speed;
static volatile int16_t left_speed_raw;
static volatile int32_t left_speed_smooth_int;
static volatile int32_t left_speed_smooth_fp;

static volatile int16_t right_speed;
static volatile int16_t right_speed_raw;
static volatile int32_t right_speed_smooth_int;
static volatile int32_t right_speed_smooth_fp;

/**
 * @brief Read left motor encoder counter.
 */
static uint16_t read_encoder_left(void) {
  return (uint16_t)timer_get_counter(TIM3);
}

/**
 * @brief Read right motor encoder counter.
 */
static uint16_t read_encoder_right(void) {
  return (uint16_t)timer_get_counter(TIM4);
}

/**
 * @brief Read left motor encoder total count.
 *
 * The total count is simply the sum of all encoder counter differences.
 */
int32_t get_encoder_left_total_ticks(void) {
  return left_total_ticks+22;
}

/**
 * @brief Read right motor encoder total count.
 *
 * The total count is simply the sum of all encoder counter differences.
 */
int32_t get_encoder_right_total_ticks(void) {
  return right_total_ticks-61;
}

int32_t get_encoder_left_absolute_position(void) {
  if (get_encoder_left_total_ticks() >= 0) {
    return get_encoder_left_total_ticks() % (MAX_ABSOLUTE_POSITION + 1);
  } else {
    return (4096 + get_encoder_left_total_ticks()) % (MAX_ABSOLUTE_POSITION + 1);
  }
}

int32_t get_encoder_right_absolute_position(void) {
  if (get_encoder_right_total_ticks() >= 0) {
    return get_encoder_right_total_ticks() % (MAX_ABSOLUTE_POSITION + 1);
  } else {
    return (4096 + get_encoder_right_total_ticks()) % (MAX_ABSOLUTE_POSITION + 1);
  }
}

void reset_encoder_left_total_ticks(void) {
  left_total_ticks = 0;
  left_diff_ticks = 0;
  set_motor_left_inited();
}

void reset_encoder_right_total_ticks(void) {
  right_total_ticks = 0;
  right_diff_ticks = 0;
  set_motor_right_inited();
}

/**
 * @brief Read left motor speed in meters per second.
 */
int32_t get_encoder_left_raw_speed(void) {
  return left_speed_raw;
}
/**
 * @brief Read left motor speed in meters per second.
 */
int32_t get_encoder_left_speed(void) {
  return left_speed_smooth_int;
}

/**
 * @brief Read right motor speed in meters per second.
 */
int32_t get_encoder_right_raw_speed(void) {
  return right_speed_raw;
}
/**
 * @brief Read right motor speed in meters per second.
 */
int32_t get_encoder_right_speed(void) {
  return right_speed_smooth_int;
}

/**
 * @brief Return the most likely counter difference.
 *
 * When reading an increasing or decreasing counter caution must be taken to:
 *
 * - Account for counter overflow.
 * - Account for counter direction (increasing or decreasing).
 *
 * This function assumes the most likely situation is to read the counter fast
 * enough to ensure that the direction is defined by the minimal difference
 * between the two reads (i.e.: readings are much faster than counter
 * overflows).
 *
 * @param[in] now Counter reading now.
 * @param[in] before Counter reading before.
 */
int32_t max_likelihood_counter_diff(uint16_t now, uint16_t before) {
  uint16_t forward_diff;
  uint16_t backward_diff;

  forward_diff = (uint16_t)(now - before);
  backward_diff = (uint16_t)(before - now);
  if (forward_diff > backward_diff)
    return -(int32_t)backward_diff;
  return (int32_t)forward_diff;
}

/**
 * @brief Update encoder readings.
 *
 * - Read raw encoder counters.
 * - Update the count differences (with respect to latest reading).
 * - Calculate distance travelled.
 * - Calculate speed.
 */
void update_encoder_readings(void) {
  static uint16_t last_left_ticks;
  static uint16_t last_right_ticks;

  // last_left_ticks = left_total_ticks;
  // last_right_ticks = right_total_ticks;

  uint16_t left_ticks;
  uint16_t right_ticks;

  left_ticks = read_encoder_left();
  right_ticks = read_encoder_right();

  left_diff_ticks = -max_likelihood_counter_diff(left_ticks, last_left_ticks);
  right_diff_ticks = max_likelihood_counter_diff(right_ticks, last_right_ticks);
  left_total_ticks += left_diff_ticks;
  right_total_ticks += right_diff_ticks;

  left_speed = (left_diff_ticks * SYSTICK_FREQUENCY_HZ * 60.0) / 4096;
  left_speed_raw = left_speed;
  left_speed <<= 3; // Shift to fixed point
  left_speed_smooth_fp = (left_speed_smooth_fp << SPEED_SMOOTHING_FACTOR) - left_speed_smooth_fp;
  left_speed_smooth_fp += left_speed;
  left_speed_smooth_fp >>= SPEED_SMOOTHING_FACTOR;
  left_speed_smooth_int = left_speed_smooth_fp >> 3;

  right_speed = (right_diff_ticks * SYSTICK_FREQUENCY_HZ * 60.0) / 4096;
  right_speed_raw = right_speed;
  right_speed <<= 3; // Shift to fixed point
  right_speed_smooth_fp = (right_speed_smooth_fp << SPEED_SMOOTHING_FACTOR) - right_speed_smooth_fp;
  right_speed_smooth_fp += right_speed;
  right_speed_smooth_fp >>= SPEED_SMOOTHING_FACTOR;
  right_speed_smooth_int = right_speed_smooth_fp >> 3;

  last_left_ticks = left_ticks;
  last_right_ticks = right_ticks;
}
