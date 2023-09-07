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
  // printf("%s\n", "PATO");
  motors_set_left_speed(90);
  motors_set_right_speed(-20);
  while (1) {
    // printf("%ld - %ld\n", get_encoder_left_total_ticks(), get_encoder_left_absolute_position());
    motors_move();
    // delay_us(1000);
    // usart_send(USART2, "o");
    // usart_send(USART2, "\n");
    // gpio_toggle(GPIOC, GPIO13);
    // printf("D: %d\tI: %d\n", get_encoder_right_total_ticks(), 0);
    // printf("%s\n", "PATO");
    // delay(100);
    // while(!gpio_get(GPIOA, GPIO4)){
    //   gpio_clear(GPIOC, GPIO13);

    //   delay(2000);
    // }/* else{
    // } */
    //   gpio_set(GPIOC, GPIO13);

    // while (!gpio_get(GPIOA, GPIO4)) {
    //   uint8_t c = spi_read(SPI1);
    //   switch (command) {
    //     case 0:
    //       command = c;
    //       break;
    //     case 3:
    //       if (c == 1) {
    //         gpio_clear(GPIOC, GPIO13);
    //       } else if (c == 2) {
    //         gpio_set(GPIOC, GPIO13);
    //       }
    //       break;
    //   }
    // }
    // command = 0;
    // gpio_set(GPIOC, GPIO13);
  }
  return 0;
}