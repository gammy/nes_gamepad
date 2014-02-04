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
#ifndef FTDI_H
#define FTDI_H

#include <ftdi.h>

struct ftdi_context *
ftdic_init(unsigned int vendor_id,
	 unsigned int product_id,
	 unsigned int baud_rate,
	 unsigned char latency,
	 unsigned int tx_buf_size,
	 unsigned int rx_buf_size);

long 
ftdic_send(struct ftdi_context *ftdic, uint8_t *buf, unsigned long s);
long
ftdic_fetch(struct ftdi_context *ftdic, uint8_t *buf, unsigned long s);
int ftdic_deinit(struct ftdi_context *ftdic);
struct ftdi_context *ftdic_connect(unsigned int vendor_id, 
				 unsigned int product_id);

#endif
