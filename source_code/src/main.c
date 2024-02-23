#include "delay.h"
#include "encoders.h"
#include "motors.h"
#include "setup.h"
#include "usart.h"
#include <libopencm3/stm32/gpio.h>

static uint32_t last_motors_update = 0;

void sys_tick_handler(void) {
  clock_tick();
  update_encoder_readings();
  // motors_move();
}

int main(void) {
  setup();
  motors_init();
  while (1) {
    if (get_clock_ticks() - last_motors_update >= 1) {
      last_motors_update = get_clock_ticks();
      motors_move();
    }
  }
  return 0;
}