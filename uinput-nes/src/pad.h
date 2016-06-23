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

#ifndef PAD_H
#define PAD_H

#include <stdint.h>

typedef struct {
    int fd;
    int num;
    uint8_t state;
    uint8_t last;
    int kbdsym[8];
} pad_t;

enum key_index {
    INDEX_A,
    INDEX_B,
    INDEX_START,
    INDEX_SELECT,
    INDEX_UP,
    INDEX_DOWN,
    INDEX_LEFT,
    INDEX_RIGHT
};

#endif
