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

