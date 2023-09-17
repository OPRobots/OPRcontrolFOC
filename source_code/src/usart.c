#include "usart.h"
#include "delay.h"
#include "setup.h"

int _write(int file, char *ptr, int len) {
  int i = 0;

  /*
   * Write "len" of char from "ptr" to file id "file"
   * Return number of char written.
   *
   * Only work for STDOUT, STDIN, and STDERR
   */
  if (file > 2) {
    return -1;
  }
  while (*ptr && (i < len)) {
    usart_send_blocking(USART3, *ptr);
    // if (*ptr == '\n') {
    // 	usart_send_blocking(USART3, '\r');
    // }
    i++;
    ptr++;
  }
  return i;
}

void manage_usart_command(char command, int value) {
  // printf("OK> %c: %d", command, value);
  switch (command) {
    case CMD_CONFIG_SINE_STEP:
      motors_set_config_sine_lookup_step(value);
      break;
    case CMD_CONFIG_SINE_UPDATE_MAX:
      motors_set_config_sine_update_max(value);
      break;
    case CMD_CONFIG_SINE_UPDATE_MIN:
      motors_set_config_sine_update_min(value);
      break;
    case CMD_MOTOR_SPEED_LEFT:
      motors_set_left_speed(value);
      break;
    case CMD_MOTOR_SPEED_RIGHT:
      motors_set_right_speed(value);
      break;
    case CMD_MOTOR_DISABLE:
      motors_disable();
      break;
    case CMD_MOTOR_ENABLE:
      motors_enable();
      break;
    default:
      printf("ERR> %c: %d", command, value);
      break;
  }
}