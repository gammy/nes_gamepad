#include "main.h"

#define IS_RIGHT(x)		((x & (1 << 0)) >> 0)
#define IS_LEFT(x)		((x & (1 << 1)) >> 1)
#define IS_DOWN(x)		((x & (1 << 2)) >> 2)
#define IS_UP(x)		((x & (1 << 3)) >> 3)
#define IS_START(x)		((x & (1 << 4)) >> 4)
#define IS_SELECT(x)		((x & (1 << 5)) >> 5)
#define IS_B(x)			((x & (1 << 6)) >> 6)
#define IS_A(x)			((x & (1 << 7)) >> 7)

volatile int busy, interrupt;

#ifdef DEBUG_UINPUT
void printbits(uint8_t b) {
	int8_t i;

	for(i = 7; i >= 0; i--) {
		printf("%d", (b & (1 << i)) >> i);
	}

	return;
}
#endif

int main(int argc, char *argv[]) {

#ifdef DEBUG_UINPUT
	unsigned long count = 0;
#endif

	int i;
	int pad_fd[2];

	const size_t bufsize = 2;

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

	ftdi_usb_purge_rx_buffer(ftdic);

	signal_install();

	busy = 1;

	while(busy) {

		uint8_t buf[2] = {0, 0};
		int r = bub_fetch(ftdic, buf, bufsize);
		if(r != bufsize)
			continue;

		for(i = 0; i < 2; i++) {

			uint8_t state = buf[i];
			int fd = pad_fd[i];

#ifdef DEBUG_UINPUT
			printf("%8ld: Pad %d: ", count, i);
			printf("%3d (%2x): ", state, state);
			printbits(state);

			printf(" Right  "); printbits(IS_RIGHT(state));
			printf(" Left   "); printbits(IS_LEFT(state));
			printf(" Up     "); printbits(IS_UP(state));
			printf(" Down   "); printbits(IS_DOWN(state));
			printf(" A      "); printbits(IS_A(state));
			printf(" B      "); printbits(IS_B(state));
			printf(" Start  "); printbits(IS_START(state));
			printf(" Select "); printbits(IS_SELECT(state));
			puts("");

			count++;
#endif

			if(IS_UP(state))
				uinput_send(fd, EV_ABS, REL_Y, -1);
			else if(IS_DOWN(state))
				uinput_send(fd, EV_ABS, REL_Y,  1);
			else
				uinput_send(fd, EV_ABS, REL_Y,  0);

			if(IS_LEFT(state))
				uinput_send(fd, EV_ABS, REL_X, -1);
			else if(IS_RIGHT(state))
				uinput_send(fd, EV_ABS, REL_X,  1);
			else
				uinput_send(fd, EV_ABS, REL_X,  0);

			uinput_send(fd, EV_KEY,  BTN_START, IS_START(state));
			uinput_send(fd, EV_KEY, BTN_SELECT, IS_SELECT(state));
			uinput_send(fd, EV_KEY,      BTN_A, IS_A(state));
			uinput_send(fd, EV_KEY,      BTN_B, IS_B(state));

			uinput_send(fd, EV_SYN, SYN_REPORT, 0);
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
