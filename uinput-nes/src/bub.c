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
 *
 * Wrapper code for serial/ftdi code (serial_, ftdic_)
 */
#include "main.h"
#include "bub.h"

bub_t *bub_init(unsigned int type) {

	bub_t *bub = malloc(sizeof(char) * sizeof(bub_t));

	if(bub == NULL) {
		fprintf(stderr, "bub_init: unable to allocate memory for bub: %s\n",
			strerror(errno));
		return(NULL);
	}

	memset(bub, 0, sizeof(char) * sizeof(bub_t));

	bub->type = type;

	return(bub);
}

int bub_connect(bub_t *bub) {

	if(bub == NULL) {
		fprintf(stderr, "bub_connect: invalid bub context passed!\n");
		return(-1);
	}

	switch(bub->type) {
		default:
			fprintf(stderr, 
				"bub_connect: invalid bub type!\n");
			return(-1);
			break;
		case BUB_TYPE_SERIAL:
			// FIXME check if we need to clean up!
			bub->serial_fd = serial_init(bub->serial_dev);

			if(bub->serial_fd < 0)
				return(-1);
			break;
		case BUB_TYPE_FTDI:
			bub->ftdic = ftdic_init(bub->ftdi_vid, 
						bub->ftdi_pid, 
						bub->baud_rate, 0, 0, 0);

			if(bub->ftdic == NULL)
				return(-1);
			break;
	}

	return(0);
}

int bub_iflush(bub_t *bub) {

	if(bub == NULL) {
		fprintf(stderr, "bub_iflush: invalid bub context passed!\n");
		return(-1);
	}

	switch(bub->type) {
		default:
			fprintf(stderr, 
				"bub_iflush: invalid bub type!\n");
			return(-1);
			break;
		case BUB_TYPE_SERIAL:
			tcflush(bub->serial_fd, TCIFLUSH);
			break;
		case BUB_TYPE_FTDI:
			ftdi_usb_purge_rx_buffer(bub->ftdic);
			break;
	}

	return(0);
}

int bub_oflush(bub_t *bub) {

	if(bub == NULL) {
		fprintf(stderr, "bub_oflush: invalid bub context passed!\n");
		return(-1);
	}

	switch(bub->type) {
		default:
			fprintf(stderr, 
				"bub_oflush: invalid bub type!\n");
			return(-1);
			break;
		case BUB_TYPE_SERIAL:
			tcflush(bub->serial_fd, TCOFLUSH);
			break;
		case BUB_TYPE_FTDI:
			ftdi_usb_purge_tx_buffer(bub->ftdic);
			break;
	}

	return(0);
}

int bub_flush(bub_t *bub) {

	if(bub == NULL) {
		fprintf(stderr, "bub_flush: invalid bub context passed!\n");
		return(-1);
	}

	switch(bub->type) {
		default:
			fprintf(stderr, 
				"bub_flush: invalid bub type!\n");
			return(-1);
			break;
		case BUB_TYPE_SERIAL:
			tcflush(bub->serial_fd, TCIOFLUSH);
			break;
		case BUB_TYPE_FTDI:
			ftdi_usb_purge_rx_buffer(bub->ftdic);
			ftdi_usb_purge_tx_buffer(bub->ftdic);
			break;
	}

	return(0);
}

int bub_send(bub_t *bub, uint8_t *buf, unsigned long s) {

	if(bub == NULL) {
		fprintf(stderr, "bub_send: invalid bub context passed!\n");
		return(-1);
	}

	switch(bub->type) {
		default:
			fprintf(stderr, 
				"bub_send: invalid bub type!\n");
			break;
		case BUB_TYPE_SERIAL:
			return(serial_send(bub->serial_fd, buf, s));
			break;
		case BUB_TYPE_FTDI:
			return(ftdic_send(bub->ftdic, buf, s));
			break;
	}

	return(-1);
}

int bub_fetch(bub_t *bub, uint8_t *buf, unsigned long s) {

	if(bub == NULL) {
		fprintf(stderr, "bub_fetch: invalid bub context passed!\n");
		return(-1);
	}

	switch(bub->type) {
		default:
			fprintf(stderr, 
				"bub_fetch: invalid bub type!\n");
			break;
		case BUB_TYPE_SERIAL:
			return(serial_fetch(bub->serial_fd, buf, s));
			break;
		case BUB_TYPE_FTDI:
			return(ftdic_fetch(bub->ftdic, buf, s));
			break;
	}

	return(-1);
}

int bub_deinit(bub_t *bub) {

	if(bub == NULL) {
		fprintf(stderr, "bub_deinit: invalid bub context passed!\n");
		return(-1);
	}

	switch(bub->type) {
		default:
			fprintf(stderr, 
				"bub_deinit: invalid bub type!\n");
			break;
		case BUB_TYPE_SERIAL:
			serial_deinit(bub->serial_fd);
			break;
		case BUB_TYPE_FTDI:
			ftdic_deinit(bub->ftdic);
			break;
	}

	return(-1);
}
