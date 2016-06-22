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
#ifndef BUB_H
#define BUB_H

#include "ftdi.h"
#include "serial.h"

enum {
    BUB_TYPE_SERIAL, 
    BUB_TYPE_FTDI
};

typedef struct {
    struct ftdi_context *ftdic;                // FTDI interface
    unsigned int        ftdi_pid, ftdi_vid;    // FTDI interface
    int                 serial_fd;             // Serial interface
    char                *serial_dev;           // Serial interface
    unsigned int        baud_rate;             // Both interfaces
    int                 type;                  
} bub_t;

bub_t * bub_init(unsigned int type);
int bub_connect(bub_t *bub);

int bub_iflush(bub_t *bub);
int bub_oflush(bub_t *bub);
int bub_flush(bub_t *bub);

int bub_send(bub_t *bub, uint8_t *buf, unsigned long s);
int bub_fetch(bub_t *bub, uint8_t *buf, unsigned long s);
int bub_deinit(bub_t *bub);

#endif
