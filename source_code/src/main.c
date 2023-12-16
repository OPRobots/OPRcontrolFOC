#include "delay.h"
#include "encoders.h"
#include "motors.h"
#include "setup.h"
#include "usart.h"

void sys_tick_handler(void) {
  clock_tick();
  update_encoder_readings();
}

int main(void) {
  setup();
  motors_init();
  motors_enable();
  motors_set_left_speed(100);
  motors_set_right_speed(100);
  while (1) {
    motors_move();

    // printf("Left: %ld | Right: %ld\n", get_encoder_left_absolute_position(), get_encoder_right_absolute_position());
    // // if(gpio_get(GPIOB, GPIO3)){

    // //  printf("azul\n");
    // // }
  }
  return 0;
}