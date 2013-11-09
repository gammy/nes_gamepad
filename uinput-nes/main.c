#include "main.h"

volatile int busy, interrupt;

int main(int argc, char *argv[]) {

	uint8_t buf[2] = {0, 0};
	uint8_t req;
	int pad_fd[2];
	int i;

	printf("Device init\n");

	for(i = 0; i < 2; i++) {

		pad_fd[i] = uinput_init();

		if(pad_fd[i] < 0)
			return(pad_fd[i]);

	}

	printf("Serial init\n");
	struct ftdi_context *ftdic = bub_init(57600, 1, 0, 0);

	if(ftdic == NULL)
		return(EXIT_FAILURE);

	ftdi_usb_purge_tx_buffer(ftdic);

	signal_install();

	busy = 1;

	while(busy) {
		buf[0] = buf[1] = 0;

		for(i = 0; i < 2; i++) {

			req = i;

			ftdi_usb_purge_rx_buffer(ftdic);

			bub_send(ftdic, &req, sizeof(req));

			int r = bub_fetch(ftdic, &buf[i], sizeof(req));
			if(r == 1)
				uinput_map_buttons(pad_fd[i], buf[i]);
			else if(r > 1)
				printf("RX overrun: %d bytes\n", r);

		}

	}

	fprintf(stderr, "Caught signal %d\n", interrupt);

	printf("Closing device\n");

	for(i = 0; i < 2; i++) {

		if(ioctl(pad_fd[i], UI_DEV_DESTROY) < 0) {
			perror("ioctl");
		}

		close(pad_fd[i]);
	}

	printf("Closing serial connection\n");

	int ret = 0;
	if ((ret = ftdi_usb_close(ftdic)) < 0) {
		fprintf(stderr, "Unable to close ftdi device: %d (%s)\n", 
			ret,
		       	ftdi_get_error_string(ftdic));
		return(EXIT_FAILURE);
	}

	ftdi_deinit(ftdic);

	return(EXIT_SUCCESS);
}
