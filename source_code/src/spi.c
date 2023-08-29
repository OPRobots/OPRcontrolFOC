#include "spi.h"

uint8_t spi_read_register(uint8_t address) {
  uint8_t reading;

  // gpio_clear(GPIOA, GPIO4);
  spi_send(SPI1, address);
  spi_read(SPI1);
  spi_send(SPI1, 0x00);
  reading = spi_read(SPI1);
  // gpio_set(GPIOA, GPIO4);

  return reading;
}

void spi_write_register(uint8_t address, uint8_t value) {
  // gpio_clear(GPIOA, GPIO4);
  spi_send(SPI1, address);
  spi_read(SPI1);
  spi_send(SPI1, value);
  spi_read(SPI1);
  // gpio_set(GPIOA, GPIO4);
  // delay(150);
}

