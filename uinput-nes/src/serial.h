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
#ifndef SERIAL_H
#define SERIAL_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#define SERIAL_PORT     "/dev/ttyACM0"
#define SERIAL_BAUDRATE     B57600 // Defined in termbits.h from termios.h
#define _POSIX_SOURCE       1

struct termios serial_oldtio, serial_newtio;

int serial_flush(int fd, int flag);
int serial_init(char *dev);
int serial_connect(char *dev);
int serial_send(int fd, uint8_t *buf, unsigned long s);
int serial_fetch(int fd, uint8_t *buf, unsigned long s);
int serial_deinit(int fd);

#endif
