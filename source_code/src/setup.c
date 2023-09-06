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
  rcc_periph_clock_enable(RCC_AFIO);

  // rcc_periph_clock_enable(RCC_SPI1);
  // rcc_periph_clock_enable(RCC_USART2);
  rcc_periph_clock_enable(RCC_USART3);

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
  // nvic_set_priority(NVIC_USART2_IRQ, 16 * 3);
  nvic_set_priority(NVIC_USART3_IRQ, 16 * 3);
  nvic_set_priority(NVIC_EXTI3_IRQ, 16 * 4);
  nvic_set_priority(NVIC_EXTI15_10_IRQ, 16 * 5);

  // nvic_enable_irq(NVIC_USART2_IRQ);
  nvic_enable_irq(NVIC_USART3_IRQ);
  // nvic_enable_irq(NVIC_SPI1_IRQ);
  nvic_enable_irq(NVIC_EXTI3_IRQ);
  nvic_enable_irq(NVIC_EXTI15_10_IRQ);
}

static void setup_gpio(void) {
  // Builtin LED
  gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);

  // Salida PWM para los motores
  gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
                GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
                GPIO0 | GPIO1 | GPIO2 | GPIO8 | GPIO9 | GPIO10);

  // Entradas Encoders
  // gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO4 | GPIO5 | GPIO6 | GPIO7);
  // gpio_set_af(GPIOB, GPIO_AF2, GPIO4 | GPIO5 | GPIO6 | GPIO7);
  // Remap TIM3 Encoders
  gpio_primary_remap(AFIO_MAPR_SWJ_CFG_JTAG_OFF_SW_ON,
                     AFIO_MAPR_TIM3_REMAP_PARTIAL_REMAP);

  // Index Encoders
  gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO3);
  gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO15);

  /* SPI */
  // gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_OPENDRAIN, GPIO4 | GPIO5 | GPIO7);
  // gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO6); // INPUT? OUTPUT 50MHZ?

  // USART2
  // gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
  //               GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART2_TX);
  // gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN,
  //               GPIO_USART2_RX);
  // gpio_set(GPIOA, GPIO_USART2_RX);

  // USART3
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
  USART_CR1(USART3) |= USART_CR1_RXNEIE;
  usart_enable(USART3);
}

void usart3_isr(void) {
  static uint8_t i = 0;
  static char command[8];

  if (((USART_CR1(USART3) & USART_CR1_RXNEIE) != 0) &&
      ((USART_SR(USART3) & USART_SR_RXNE) != 0)) {

    uint8_t data = usart_recv(USART3);
    if (data != '\n') {
      command[i++] = data;
    } else {
      manage_usart_command(command[0], atoi(&command[1]));
      for (uint8_t p = 0; p < i; p++) {
        command[p] = '\0';
      }
      i = 0;
    }
  }
}

static void setup_quadrature_encoders(void) {
  timer_set_period(TIM4, 0xFFFF);
  timer_slave_set_mode(TIM4, TIM_SMCR_SMS_EM3);
  timer_ic_set_input(TIM4, TIM_IC1, TIM_IC_IN_TI1);
  timer_ic_set_input(TIM4, TIM_IC2, TIM_IC_IN_TI2);
  timer_enable_counter(TIM4);

  exti_select_source(EXTI3, GPIOB);
  exti_set_trigger(EXTI3, EXTI_TRIGGER_FALLING);
  exti_enable_request(EXTI3);

  timer_set_period(TIM3, 0xFFFF);
  timer_slave_set_mode(TIM3, TIM_SMCR_SMS_EM3);
  timer_ic_set_input(TIM3, TIM_IC1, TIM_IC_IN_TI1);
  timer_ic_set_input(TIM3, TIM_IC2, TIM_IC_IN_TI2);
  timer_enable_counter(TIM3);

  exti_select_source(EXTI15, GPIOA);
  exti_set_trigger(EXTI15, EXTI_TRIGGER_FALLING);
  exti_enable_request(EXTI15);
}

void exti3_isr(void) {
  reset_encoder_right_total_ticks();
  exti_reset_request(EXTI3);
}

void exti15_10_isr(void) {
  reset_encoder_left_total_ticks();
  exti_reset_request(EXTI15);

}

static void setup_pwm(void) {
  timer_set_mode(TIM1, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE,
                 TIM_CR1_DIR_UP);

  timer_set_prescaler(TIM1, 4);
  timer_set_repetition_counter(TIM1, 0);
  timer_enable_preload(TIM1);
  timer_continuous_mode(TIM1);
  timer_set_period(TIM1, PWM_PERIOD);

  timer_set_oc_mode(TIM1, TIM_OC1, TIM_OCM_PWM1);
  timer_set_oc_mode(TIM1, TIM_OC2, TIM_OCM_PWM1);
  timer_set_oc_mode(TIM1, TIM_OC3, TIM_OCM_PWM1);
  timer_set_oc_mode(TIM1, TIM_OC4, TIM_OCM_PWM1);
  timer_set_oc_value(TIM1, TIM_OC1, 0);
  timer_set_oc_value(TIM1, TIM_OC2, 0);
  timer_set_oc_value(TIM1, TIM_OC3, 0);
  timer_enable_oc_output(TIM1, TIM_OC1);
  timer_enable_oc_output(TIM1, TIM_OC2);
  timer_enable_oc_output(TIM1, TIM_OC3);
  timer_disable_oc_output(TIM1, TIM_OC4);
  timer_enable_break_main_output(TIM1);

  timer_enable_counter(TIM1);

  timer_set_mode(TIM1, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE,
                 TIM_CR1_DIR_UP);

  timer_set_prescaler(TIM2, 4);
  timer_set_repetition_counter(TIM2, 0);
  timer_enable_preload(TIM2);
  timer_continuous_mode(TIM2);
  timer_set_period(TIM2, PWM_PERIOD);

  timer_set_oc_mode(TIM2, TIM_OC1, TIM_OCM_PWM1);
  timer_set_oc_mode(TIM2, TIM_OC2, TIM_OCM_PWM1);
  timer_set_oc_mode(TIM2, TIM_OC3, TIM_OCM_PWM1);
  timer_set_oc_mode(TIM2, TIM_OC4, TIM_OCM_PWM1);
  timer_set_oc_value(TIM2, TIM_OC1, 0);
  timer_set_oc_value(TIM2, TIM_OC2, 0);
  timer_set_oc_value(TIM2, TIM_OC3, 0);
  timer_enable_oc_output(TIM2, TIM_OC1);
  timer_enable_oc_output(TIM2, TIM_OC2);
  timer_enable_oc_output(TIM2, TIM_OC3);
  timer_disable_oc_output(TIM2, TIM_OC4);
  // timer_enable_break_main_output(TIM2);

  timer_enable_counter(TIM2);
}

// static void setup_spi(void) {
//   spi_reset(SPI1);
//   // spi_disable_crc(SPI1);

//   // spi_init_slave
//   uint32_t reg32 = SPI_CR1(SPI1);

//   /* Reset all bits omitting SPE, CRCEN and CRCNEXT bits. */
//   reg32 &= SPI_CR1_SPE | SPI_CR1_CRCEN | SPI_CR1_CRCNEXT;

//   reg32 |= (0 << 2); /* Configure SPI as slave. */

//   reg32 |= SPI_CR1_BAUDRATE_FPCLK_DIV_64;   /* Set baud rate bits. */
//   reg32 |= SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE; /* Set CPOL value. */
//   reg32 |= SPI_CR1_CPHA_CLK_TRANSITION_2;   /* Set CPHA value. */
//   reg32 |= SPI_CR1_DFF_8BIT;                /* Set data format (8 or 16 bits). */
//   reg32 |= SPI_CR1_MSBFIRST;                /* Set frame format (LSB- or MSB-first). */
//   reg32 |= SPI_CR1_BIDIMODE;                /* Set Bidirectional data mode enable. */

//   SPI_CR2(SPI1) |= SPI_CR2_SSOE; /* common case */
//   SPI_CR1(SPI1) = reg32;

//   spi_disable_software_slave_management(SPI1);
//   spi_set_nss_high(SPI1);

//   spi_enable(SPI1);
// }

void setup(void) {
  setup_clock();
  setup_gpio();
  setup_usart();
  setup_pwm();
  setup_quadrature_encoders();

  setup_timer_priorities();
  // setup_spi();
  setup_systick();
}