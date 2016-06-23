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
#include "pad.h"

#define UINPUT_MODE_JOYSTICK            0
#define UINPUT_MODE_JOYSTICK_NO_AXIS    1
#define UINPUT_MODE_KEYBOARD            2

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

int uinput_init(pad_t *pad, int mode);
void uinput_deinit(pad_t *pad);
int  uinput_send(pad_t *pad, uint16_t type, uint16_t code, int32_t val);
void uinput_map(pad_t *pad, int buttons_only);

#endif
