#include "setup.h"
#include "usart.h"
#include "delay.h"
#include "encoders.h"

void sys_tick_handler(void) {
  clock_tick();
  update_encoder_readings();
}

int main(void) {
  setup();


  while (1) {
    gpio_toggle(GPIOC, GPIO13);
    printf("D: %d\tI: %d\n", get_encoder_right_total_ticks(), 0);
    delay(100);
  }
  return 0;
}