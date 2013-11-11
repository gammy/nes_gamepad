#ifndef UINPUT_H
#define UINPUT_H

#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>

#define IS_RIGHT(x)	((x & (1 << 0)) >> 0)
#define IS_LEFT(x)	((x & (1 << 1)) >> 1)
#define IS_DOWN(x)	((x & (1 << 2)) >> 2)
#define IS_UP(x)	((x & (1 << 3)) >> 3)
#define IS_START(x)	((x & (1 << 4)) >> 4)
#define IS_SELECT(x)	((x & (1 << 5)) >> 5)
#define IS_B(x)		((x & (1 << 6)) >> 6)
#define IS_A(x)		((x & (1 << 7)) >> 7)

// Derived from reverse-engineering a generic USB gamepad;
// This shouldn't be needed. -1 to 1 should be fine since that's
// what all of the gamecon drivers use (see src/linux/input/joystick/gamecon.c)
#define AXIS_MIN        -32767
#define AXIS_MAX         32767

int uinput_init(int device_number);
int uinput_send(int fd, uint16_t type, uint16_t code, int32_t val);
void uinput_map_buttons(int fd, uint8_t state);

#endif
