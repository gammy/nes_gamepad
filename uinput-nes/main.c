#include "main.h"

#define IS_RIGHT(x)		(x & 0b00000001)
#define IS_LEFT(x)		(x & 0b00000010)
#define IS_DOWN(x)		(x & 0b00000100)
#define IS_UP(x)		(x & 0b00001000)
#define IS_START(x)		(x & 0b00010000)
#define IS_SELECT(x)		(x & 0b00100000)
#define IS_B(x)			(x & 0b01000000)
#define IS_A(x)			(x & 0b10000000)

volatile int busy, interrupt;

int main(int argc, char *argv[]) {

	int i;
	uint8_t pads[2];
	const size_t bufsize = 2; //sizeof(pads);

	printf("Device init\n");
	int fd = uinput_init();
	if(fd < 0)
		return(fd);

	printf("Serial init\n");
	struct ftdi_context *ftdic = bub_init(57600, 1, 0, 4096);

	if(ftdic == NULL)
		return(EXIT_FAILURE);

	memset(&pads, 0, bufsize);
	ftdi_usb_purge_rx_buffer(ftdic);

	signal_install();

	busy = 1;

	while(busy) {

		int r = bub_fetch(ftdic, pads, bufsize);
		if(r != bufsize)
			continue;

		//for(i = 0; i < 2; i++) {

			uint8_t pad = pads[0];

			if(pad == 0) {
				usleep(100);
				continue;
			}

			if(IS_UP(pad))
				uinput_send(fd, EV_ABS, REL_Y, -1);
			else if(IS_DOWN(pad))
				uinput_send(fd, EV_ABS, REL_Y,  1);
			else
				uinput_send(fd, EV_ABS, REL_Y,  0);

			if(IS_LEFT(pad))
				uinput_send(fd, EV_ABS, REL_X, -1);
			else if(IS_RIGHT(pad))
				uinput_send(fd, EV_ABS, REL_X,  1);
			else
				uinput_send(fd, EV_ABS, REL_X,  0);

			uinput_send(fd, EV_KEY,  BTN_START, IS_START(pad));
			uinput_send(fd, EV_KEY, BTN_SELECT, IS_SELECT(pad));
			uinput_send(fd, EV_KEY,      BTN_A, IS_A(pad));
			uinput_send(fd, EV_KEY,      BTN_B, IS_B(pad));

			uinput_send(fd, EV_SYN, SYN_REPORT, 0);
		//}

	}

	fprintf(stderr, "Caught signal %d\n", interrupt);

	printf("Closing device\n");

	if(ioctl(fd, UI_DEV_DESTROY) < 0) {
		perror("ioctl");
	}

	close(fd);

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
