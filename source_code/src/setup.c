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
  rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_84MHZ]);

  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_GPIOB);
  rcc_periph_clock_enable(RCC_GPIOC);

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
  nvic_set_priority(NVIC_USART2_IRQ, 16 * 2);

  nvic_enable_irq(NVIC_USART2_IRQ);
}

static void setup_gpio(void) {
  // Builtin LED
  gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO13);

  // Salida PWM para los motores
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO8 | GPIO9 | GPIO10 | GPIO11);
  gpio_set_af(GPIOA, GPIO_AF2, GPIO8 | GPIO9 | GPIO10 | GPIO11);
  gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO3 | GPIO5);
  gpio_set_af(GPIOB, GPIO_AF2, GPIO3 | GPIO5);

  // Entradas Encoders
  gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6 | GPIO7);
  gpio_set_af(GPIOB, GPIO_AF2, GPIO6 | GPIO7);
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6 | GPIO7);
  gpio_set_af(GPIOA, GPIO_AF2, GPIO6 | GPIO7);

  // USART2
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO1 | GPIO2);
  gpio_set_af(GPIOA, GPIO_AF7, GPIO1 | GPIO2);
}

static void setup_usart(void) {
  usart_set_baudrate(USART2, 115200);
  usart_set_databits(USART2, 8);
  usart_set_stopbits(USART2, USART_STOPBITS_1);
  usart_set_parity(USART2, USART_PARITY_NONE);
  usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);
  usart_set_mode(USART2, USART_MODE_TX_RX);
  // USART_CR1(USART2) |= USART_CR1_RXNEIE;
  // usart_enable_tx_interrupt(USART2);
  usart_enable(USART2);
}

static void setup_quadrature_encoders() {
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

void setup(void) {
  setup_clock();
  setup_gpio();
  setup_usart();
  setup_quadrature_encoders();

  setup_timer_priorities();
  setup_systick();
}