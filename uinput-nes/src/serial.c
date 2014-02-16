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
#include "serial.h"

// Code is basically the same as 
// http://tldp.org/HOWTO/Serial-Programming-HOWTO/x115.html

int serial_init(char *dev) {

	int fd;

	if(verbosity > 0) {
		fprintf(stderr, "Opening serial device %s\n", dev);
	}

	fd = open(dev, O_RDWR | O_NOCTTY); 
	if(fd < 0) {
		fprintf(stderr, "Unable to open %s: %s\n", 
			dev, 
			strerror(errno));
		return(fd);
	}

	tcgetattr(fd, &serial_oldtio); /* save current serial port settings */
	bzero(&serial_newtio, sizeof(serial_newtio)); /* clear struct for new port settings */

	/* 
	BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
	CRTSCTS : output hardware flow control (only used if the cable has
	all necessary lines. See sect. 7 of Serial-HOWTO)
	CS8     : 8n1 (8bit,no parity,1 stopbit)
	CLOCAL  : local connection, no modem contol
	CREAD   : enable receiving characters
	 */
	//newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;

	serial_newtio.c_cflag = SERIAL_BAUDRATE | CS8 | CLOCAL | CREAD;
	serial_newtio.c_iflag = IGNPAR; // Ignore bytes with parity errors
	serial_newtio.c_oflag = 0; // Raw output
	serial_newtio.c_lflag = 0; // Non-canonical
	//serial_newtio.c_lflag = ICANON; // Non-canonical
	serial_newtio.c_cc[VTIME]    = 0;     /* inter-character timer unused */
	serial_newtio.c_cc[VMIN]     = 2;     /* blocking read until character(s) arrive */

	tcflush(fd, TCIFLUSH);

	int ret = tcsetattr(fd, TCSANOW, &serial_newtio);
	if(ret < 0) {
		fprintf(stderr, "serial_init tcsetattr: %s\n", 
			strerror(errno));
		return(errno);
	}

	return(fd);
}

int serial_send(int fd, uint8_t *buf, unsigned long s) {

	if(fd < 0) {
		fprintf(stderr, "serial_send: Invalid file descriptor passed!\n");
		return(-1);
	}

	if(verbosity > 1)
		fprintf(stderr, "SENDING %ld BYTES\n", s);

	ssize_t txb = write(fd, buf, s);

	if(txb < 0){
		fprintf(stderr, "\nTX Error: %ld: %s\n", 
			txb, 
			strerror(errno));
	} else if(txb > 0 && txb != s) {
		fprintf(stderr, "\nTX under or overrun (%ld bytes): %s\n", 
			txb, 
			strerror(errno));
	}

	if(verbosity > 1)
		fprintf(stderr, "SENT %ld BYTES\n", txb);

	return(txb);
}

int serial_fetch(int fd, uint8_t *buf, unsigned long s) {

	if(fd < 0) {
		fprintf(stderr, "serial_fetch: Invalid file descriptor passed!\n");
		return(-1);
	}

	if(verbosity > 1)
		fprintf(stderr, "FETCHING %ld BYTES\n", s);

	ssize_t rxb = read(fd, buf, s);

	if(rxb < 0) {
		fprintf(stderr, "\nRX Error: %ld: %s\n", 
			rxb, 
			strerror(errno));
	}

	if(verbosity > 1)
		fprintf(stderr, "FETCHED %ld BYTES\n", rxb);

	return(rxb);
}

int serial_deinit(int fd) {

	if(fd < 0) {
		fprintf(stderr, "serial_deinit: Invalid file descriptor passed!\n");
		return(-1);
	}

	tcflush(fd, TCIOFLUSH);
	tcsetattr(fd, TCSANOW, &serial_oldtio);
	close(fd);

	return(0);
}

#if 0
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
#endif


int serial_connect(char *dev) {

	int fd = serial_init(dev);

	if(fd >= 0)
		tcflush(fd, TCIOFLUSH);

	return(fd);
}
