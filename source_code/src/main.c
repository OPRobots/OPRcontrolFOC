#include "delay.h"
#include "encoders.h"
#include "motors.h"
#include "setup.h"
#include "usart.h"
#include <libopencm3/stm32/gpio.h>

void sys_tick_handler(void) {
  clock_tick();
  update_encoder_readings();
  motors_move();
}

int main(void) {
  setup();
  motors_init();
  // motors_enable();
  // motors_set_right_speed(40);
  // motors_set_left_speed(40);
  while (1) {
    //
    // delay(1000);
    // gpio_toggle(GPIOC, GPIO14);

    // 0 - 586
    // int32_t pos = get_encoder_left_absolute_position();
    // int16_t speed = 0;
    // if (pos <= 100) {
    //   speed = 0;
    // } else if (pos > 100 && pos <= 200) {
    //   speed = 30;
    // } else if (pos > 200 && pos <= 300) {
    //   speed = 60;
    // } else if (pos > 300 && pos <= 400) {
    //   speed = 10;
    // } else if (pos > 400 && pos <= 500) {
    //   speed = 80;
    // } else if (pos > 500 && pos <= 600) {
    //   speed = 100;
    // }

    // motors_set_left_speed(40);

    // printf("%ld %d %d 0 100\n", (int32_t)map(abs(get_encoder_left_speed()), 0, 900, 0, 100), motor_left_get_speed_factor(), 40);

    // delay(150);
  }
  return 0;
}