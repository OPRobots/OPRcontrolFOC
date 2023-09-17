#ifndef __COMMAND_H
#define __COMMAND_H

#include "commands.h"
#include "motors.h"
#include "usart.h"


void manage_command(char command, int value);
void send_command(char command, int value);

#endif