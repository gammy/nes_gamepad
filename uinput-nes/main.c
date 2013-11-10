#include "main.h"
#include <getopt.h>
#include <libgen.h>

volatile int busy, interrupt;

#define VERSION	"Dev"

void usage(char *me) {
	printf("uinput-nes v%s by gammy\n"
	       "Usage: %s [options]\n\n"
	       "-p  --pads <num>      Simulate <num> joypads (default: 1, max: 4)\n"
	       "-h  --help            This help\n"
	       "-V  --version         Display version\n\n"
	       "This is free software; see the source for copying conditions. "
	       "There is NO\nwarranty; not even for MERCHANTABILITY or FITNESS "
	       "FOR A PARTICULAR PURPOSE.\n\n",
	       VERSION, me);
}


int main(int argc, char *argv[]) {

	static struct option const long_options[] = {
		{"pads",       required_argument, NULL, 'p'},
		{"version",    no_argument,       NULL, 'V'},
		{"help",       no_argument,       NULL, 'h'},
		{NULL,         0,                 NULL,  0}
	};

	uint8_t buf[2] = {0, 0};
	uint8_t req;
	int pad_fd[2];
	int i;

	int numpads = 1;

	while((i = getopt_long(argc, argv, "p:Vh", long_options, NULL)) != -1){
		switch(i) {
			case 'p':
				numpads = atoi(optarg);
				break;
			case 'h':
				usage(basename(argv[0]));
				return(EXIT_SUCCESS);
			case 'V':
				printf("uinput-nes v%s by gammy\n", VERSION);
				return(EXIT_SUCCESS);
			default:
				break;
		}
	}

	if(numpads < 1 || numpads > 4) {
		fprintf(stderr, "Only 1-4 gamepads can be used\n");
		return(EXIT_FAILURE);
	}

	printf("Initializing %d gamepad%s\n", 
	       numpads, 
	       numpads == 1 ? "" : "s");

	for(i = 0; i < numpads; i++) {

		pad_fd[i] = uinput_init(1 + i);

		if(pad_fd[i] < 0)
			return(pad_fd[i]);

	}

	printf("Initializing serial interface\n");
	struct ftdi_context *ftdic = bub_init(57600, 1, 0, 0);

	if(ftdic == NULL)
		return(EXIT_FAILURE);

	ftdi_usb_purge_tx_buffer(ftdic);

	signal_install();

	busy = 1;

	while(busy) {
		buf[0] = buf[1] = 0;

		for(i = 0; i < numpads; i++) {

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

	printf("Closing gamepad device%s\n", numpads == 1 ? "" : "s");

	for(i = 0; i < 2; i++) {

		if(ioctl(pad_fd[i], UI_DEV_DESTROY) < 0) {
			perror("ioctl");
		}

		close(pad_fd[i]);
	}

	printf("Closing serial interface\n");

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
