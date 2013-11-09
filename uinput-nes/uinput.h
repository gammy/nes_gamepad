#ifndef UINPUT_H
#define UINPUT_H

#include "main.h"

int uinput_init(void);
int uinput_send(int fd, uint16_t type, uint16_t code, int32_t val);

#endif
