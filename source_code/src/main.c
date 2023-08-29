#include "delay.h"
#include "encoders.h"
#include "setup.h"
#include "spi.h"
#include "usart.h"

void sys_tick_handler(void) {
  clock_tick();
  update_encoder_readings();
}

uint8_t command = 0;

int main(void) {
  setup();

  while (1) {
    // usart_send(USART2, "o");
    // usart_send(USART2, "\n");
    // gpio_toggle(GPIOC, GPIO13);
    // printf("D: %d\tI: %d\n", get_encoder_right_total_ticks(), 0);
    printf("%s\n", "PATO");
    delay(100);
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