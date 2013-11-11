/* This file is part of uinput-nes.
 *
 * uinput-nes is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * uinput-nes is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with uinput-nes.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef UINPUT_H
#define UINPUT_H

#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>

#define IS_RIGHT(x)     ((x & (1 << 0)) >> 0)
#define IS_LEFT(x)      ((x & (1 << 1)) >> 1)
#define IS_DOWN(x)      ((x & (1 << 2)) >> 2)
#define IS_UP(x)        ((x & (1 << 3)) >> 3)
#define IS_START(x)     ((x & (1 << 4)) >> 4)
#define IS_SELECT(x)    ((x & (1 << 5)) >> 5)
#define IS_B(x)         ((x & (1 << 6)) >> 6)
#define IS_A(x)         ((x & (1 << 7)) >> 7)

// Derived from reverse-engineering a generic USB gamepad;
// This shouldn't be needed. -1 to 1 should be fine since that's
// what all of the gamecon drivers use (see src/linux/input/joystick/gamecon.c)
#define AXIS_MIN        -32767
#define AXIS_MAX         32767

typedef struct {
	int fd;
	int num;
	uint8_t state;
	uint8_t last;
} pad_t;

int  uinput_init(int device_number, int buttons_only);
void uinput_deinit(pad_t *pad);
int  uinput_send(int fd, uint16_t type, uint16_t code, int32_t val);
void uinput_map(pad_t *pad, int buttons_only);

#endif
