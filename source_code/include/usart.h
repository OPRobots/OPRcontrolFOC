#ifndef __USART_H
#define __USART_H

#include <libopencm3/stm32/usart.h>

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <unistd.h>

#include "commands.h"
#include "motors.h"

int _write(int file, char *ptr, int len);

void manage_usart_command(char command, int value);

#endif