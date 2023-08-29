#include "setup.h"

/**
 * @brief Configura los relojes principales del robot
 *
 * SYSCLK a 84 MHz
 * GPIO: A, B, C
 * USART3
 * Timers: 1-PWM, 4-ADDR_LED
 *
 */
static void setup_clock(void) {
  rcc_clock_setup_in_hse_8mhz_out_72mhz();

  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_GPIOB);
  rcc_periph_clock_enable(RCC_GPIOC);

  rcc_periph_clock_enable(RCC_SPI1);
  rcc_periph_clock_enable(RCC_USART2);

  rcc_periph_clock_enable(RCC_TIM1);
  rcc_periph_clock_enable(RCC_TIM2);
  rcc_periph_clock_enable(RCC_TIM3);
  rcc_periph_clock_enable(RCC_TIM4);

  dwt_enable_cycle_counter();
}

/**
 * @brief Configura el SysTick para 1ms
 *
 */
static void setup_systick(void) {
  systick_set_frequency(SYSTICK_FREQUENCY_HZ, SYSCLK_FREQUENCY_HZ);
  systick_counter_enable();
  systick_interrupt_enable();
}

static void setup_timer_priorities(void) {
  nvic_set_priority(NVIC_SYSTICK_IRQ, 16 * 1);
  nvic_set_priority(NVIC_SPI1_IRQ, 16 * 2);
  nvic_set_priority(NVIC_USART2_IRQ, 16 * 3);

  nvic_enable_irq(NVIC_USART2_IRQ);
  // nvic_enable_irq(NVIC_SPI1_IRQ);
}

static void setup_gpio(void) {
  // Builtin LED
  gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);

  // Salida PWM para los motores
  gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
                GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO8 | GPIO9 | GPIO10 | GPIO11);
  gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
                GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO3 | GPIO5);

  // Entradas Encoders
  // gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6 | GPIO7);
  // gpio_set_af(GPIOB, GPIO_AF2, GPIO6 | GPIO7);
  // gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6 | GPIO7);
  // gpio_set_af(GPIOA, GPIO_AF2, GPIO6 | GPIO7);

  /* SPI */
  gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_OPENDRAIN, GPIO4 | GPIO5 | GPIO7);
  gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO6); // INPUT? OUTPUT 50MHZ?

  // USART2
  gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
                GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART3_TX);
  gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN,
                GPIO_USART3_RX);
  gpio_set(GPIOB, GPIO_USART3_RX);
}

static void setup_usart(void) {
  usart_set_baudrate(USART3, 115200);
  usart_set_databits(USART3, 8);
  usart_set_stopbits(USART3, USART_STOPBITS_1);
  usart_set_parity(USART3, USART_PARITY_NONE);
  usart_set_flow_control(USART3, USART_FLOWCONTROL_NONE);
  usart_set_mode(USART3, USART_MODE_TX_RX);
  // USART_CR1(USART3) |= USART_CR1_RXNEIE;
  // usart_enable_tx_interrupt(USART3);
  usart_enable(USART3);
}

static void setup_quadrature_encoders(void) {
  timer_set_period(TIM4, 0xFFFF);
  timer_slave_set_mode(TIM4, TIM_SMCR_SMS_EM3);
  timer_ic_set_input(TIM4, TIM_IC1, TIM_IC_IN_TI1);
  timer_ic_set_input(TIM4, TIM_IC2, TIM_IC_IN_TI2);
  timer_enable_counter(TIM4);

  timer_set_period(TIM3, 0xFFFF);
  timer_slave_set_mode(TIM3, TIM_SMCR_SMS_EM3);
  timer_ic_set_input(TIM3, TIM_IC1, TIM_IC_IN_TI1);
  timer_ic_set_input(TIM3, TIM_IC2, TIM_IC_IN_TI2);
  timer_enable_counter(TIM3);
}

static void setup_spi(void) {
  spi_reset(SPI1);
  // spi_disable_crc(SPI1);

  // spi_init_slave
  uint32_t reg32 = SPI_CR1(SPI1);

  /* Reset all bits omitting SPE, CRCEN and CRCNEXT bits. */
  reg32 &= SPI_CR1_SPE | SPI_CR1_CRCEN | SPI_CR1_CRCNEXT;

  reg32 |= (0 << 2); /* Configure SPI as slave. */

  reg32 |= SPI_CR1_BAUDRATE_FPCLK_DIV_64;   /* Set baud rate bits. */
  reg32 |= SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE; /* Set CPOL value. */
  reg32 |= SPI_CR1_CPHA_CLK_TRANSITION_2;   /* Set CPHA value. */
  reg32 |= SPI_CR1_DFF_8BIT;                /* Set data format (8 or 16 bits). */
  reg32 |= SPI_CR1_MSBFIRST;                /* Set frame format (LSB- or MSB-first). */
  reg32 |= SPI_CR1_BIDIMODE;                /* Set Bidirectional data mode enable. */

  SPI_CR2(SPI1) |= SPI_CR2_SSOE; /* common case */
  SPI_CR1(SPI1) = reg32;

  spi_disable_software_slave_management(SPI1);
  spi_set_nss_high(SPI1);

  spi_enable(SPI1);
}

void setup(void) {
  setup_clock();
  setup_gpio();
  setup_usart();
  setup_quadrature_encoders();

  setup_timer_priorities();
  setup_spi();
  setup_systick();
}