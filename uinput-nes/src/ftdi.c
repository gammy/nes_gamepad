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
#include "main.h"
#include "ftdi.h"

// FTDI routines taken from an older project,
// https://github.com/gammy/JeePointer
struct ftdi_context *
bub_init(unsigned int baud_rate,
	 unsigned char latency,
	 unsigned int tx_buf_size,
	 unsigned int rx_buf_size) {

	int ret = 0;

	struct ftdi_context *ftdic;
	
	ftdic = malloc(sizeof(struct ftdi_context));
   
	if(ftdic == NULL) {
		perror("malloc");
		return(NULL);
	}

	ret = ftdi_init(ftdic);
	if (ret < 0) {
		fprintf(stderr, "ftdi_init failed: %d.\n", ret);
		return(NULL);
	}

	ftdi_set_interface(ftdic, INTERFACE_ANY);

	ret = ftdi_usb_open(ftdic, 0x0403, 0x6001); // FIXME make nice defines
	if(ret < 0) {
		fprintf(stderr, "Unable to open ftdi device: %d (%s)\n", 
			ret, ftdi_get_error_string(ftdic));
		return(NULL);
	}

	if(ftdi_usb_reset(ftdic) != 0)
		fprintf(stderr, "WARN: ftdi_usb_reset failed!\n");

	ftdi_disable_bitbang(ftdic);

	if (ftdi_set_baudrate(ftdic, baud_rate) < 0) {
		fprintf(stderr, "Unable to set baudrate: (%s)\n", 
			ftdi_get_error_string(ftdic));
		return(NULL);
	} 

	ftdi_set_latency_timer(ftdic, latency);

	if(tx_buf_size > 0)
		ftdi_write_data_set_chunksize(ftdic, tx_buf_size);
	if(rx_buf_size > 0)
		ftdi_read_data_set_chunksize(ftdic, rx_buf_size);

	return(ftdic);
}

// XXX Hacked together :p not very smart
long bub_send(struct ftdi_context *ftdic, uint8_t *buf, unsigned long s) {

	if(ftdic == NULL) {
		fprintf(stderr, "bub_send: Invalid ftdi context passed!\n");
		return(-1);
	}

#if 0
	unsigned long i;
	for(i = 0; i < s; i++)
		printf("bub_send: Send byte %ld/%ld: %d\n", 1+i, s, buf[i]);
#endif

	unsigned long txb = ftdi_write_data(ftdic, buf, s);

	if(txb < 0){
		fprintf(stderr, "\nTX Error: %ld: %s\n", 
			txb, 
			ftdi_get_error_string(ftdic));
	} else if(txb > 0 && txb != s) {
		fprintf(stderr, "\nTX under or overrun (%ld bytes): %s\n", 
			txb, 
			ftdi_get_error_string(ftdic));
	}

	return(txb);

}

/* Nonblocking fetch, guarantees complete packet or nothing/error.
 * Not reentrant (thread) safe
 *
 * @param struct ftdi_context *	FTDI context pointer
 * @param uint8_t *		Pointer to buffer
 * @param unsigned long		Size of buffer
 * @returns			Size of buffer(success), 
 *                              0(buffer not filled yet) or 
 *                              error(negative value)
 */
long
bub_fetch(struct ftdi_context *ftdic, uint8_t *buf, unsigned long s) {

	static unsigned long rxb = 0;
	static unsigned long offs = 0;

	if(ftdic == NULL) {
		fprintf(stderr, "bub_fetch: Invalid ftdi context passed!\n");
		return(-1);
	}

	if(s - offs < 0)
		abort();

	if(offs != 0)
		fprintf(stderr, "Asking for %ld\n", s - offs);

	rxb = ftdi_read_data(ftdic, &buf[offs], s - offs);

	if(offs != 0 && rxb != 0)
		fprintf(stderr, "Got %ld\n", rxb);

	if(rxb != 0) {

		if(rxb < 0){
			fprintf(stderr, "\nRX Error: %ld: %s\n", rxb, ftdi_get_error_string(ftdic));
			memset(buf, 0, s);
			offs = 0;
			return(rxb);
		}

		if(rxb + offs < s) {
			offs = rxb;
			fprintf(stderr, "\nRX Underrun: Got %ld, need %ld more\n", rxb, s - offs);
		} else if(rxb + offs > s) {
			fprintf(stderr, "\nRX Overrun, %ld too much(rxb=%ld, offs=%ld)\n", 
				s - offs,
				rxb,
				offs);
			memset(buf, 0, s);
			offs = 0;
		}
	} 

	if(rxb + offs == s) {
		offs = 0;
		return(s);
	} 

	return(0);
}

int bub_deinit(struct ftdi_context *ftdic) {

	if(ftdic == NULL) {
		fprintf(stderr, "bub_deinit: Invalid ftdi context passed!\n");
		return(-1);
	}

	int ret = 0;

	if ((ret = ftdi_usb_close(ftdic)) < 0) {
		fprintf(stderr, "Unable to close ftdi device: %d (%s)\n", 
			ret,
		       	ftdi_get_error_string(ftdic));
		return(EXIT_FAILURE);
	}

	ftdi_deinit(ftdic);
	free(ftdic);

	ftdic = NULL;

	return(EXIT_SUCCESS);
}

struct ftdi_context *bub_connect(void) {

	struct ftdi_context *ftdic = bub_init(57600, 1, 0, 0);

	if(ftdic == NULL)
		return(NULL);

	ftdi_usb_purge_tx_buffer(ftdic);

	return(ftdic);
}

