#include "delay.h"
#include "encoders.h"
#include "motors.h"
#include "setup.h"
#include "spi.h"
#include "usart.h"

void sys_tick_handler(void) {
  clock_tick();
  update_encoder_readings();
}

int main(void) {
  setup();
  motors_init();
  while (1) {
    motors_move();
  }
  return 0;
}