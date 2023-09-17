#include "command.h"

void manage_command(char command, int value) {
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
      // TODO: show error on LEDs
      // printf("ERR> %c: %d", command, value);
      break;
  }
}

void send_command(char command, int value) {
  printf("%c%d\n", command, value);
}