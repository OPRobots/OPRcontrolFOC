#include "delay.h"
#include "encoders.h"
#include "motors.h"
#include "setup.h"
#include "usart.h"
#include <libopencm3/stm32/gpio.h>

static uint32_t last_print_ms = 0;

void sys_tick_handler(void) {
  clock_tick();
  update_encoder_readings();
  motors_move();
}

int main(void) {
  setup();
  motors_init();
  motors_enable();
  motors_set_left_speed(100);
  motors_set_right_speed(100);
  while (1) {
    // 
    // delay(1000);
    // gpio_toggle(GPIOC, GPIO14);

    // if (get_clock_ticks() - last_print_ms > 1000) {
      // printf("Left: %ld | Right: %ld\n", get_encoder_left_absolute_position(), get_encoder_right_absolute_position());
      printf("-800 %ld %ld -1200\n", get_encoder_left_speed(), get_encoder_left_raw_speed());
      // delay(150);
      // last_print_ms = get_clock_ticks();
    // }
  }
  return 0;
}