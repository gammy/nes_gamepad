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
#ifndef KBD_H
#define KBD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <linux/input-event-codes.h>
#include "pad.h"

struct kbd_keyval {
    char *name;
    int code;
};

void kbd_print_keys(void);
int kbd_get_code(char *target_name);
//int kbdopt_parse(char *opts, int **keymap);
//int kbdopt_parse(char *opts, int *keymap[8], size_t keymap_elems);
int kbdopt_parse(char *opts, pad_t *pads, size_t num_pads);
#endif
