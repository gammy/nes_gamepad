#include "main.h"
#include <getopt.h>
#include <libgen.h>

int verbosity = 0;
volatile int busy, interrupt;

#define VERSION	"Dev"

void usage(char *me) {
	printf("uinput-nes v%s by gammy\n"
	       "Usage: %s [options]\n\n"
	       "-p  --pads <number>   Simulate <number> joypads           (default: 1, max: 4)\n"
	       "-v  --verbose <level> Verbosity level <level>             (default: 0, max: 2)\n"
	       "-n  --noaxis          Emulate D-pad with buttons                (default: off)\n"
	       "-P  --passthrough     Pass through data, not just state changes (default: off)\n"
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
		{"verbose",    required_argument, NULL, 'v'},
		{"noaxis",     no_argument,       NULL, 'n'},
		{"passthrough",no_argument,       NULL, 'P'},
		{"version",    no_argument,       NULL, 'V'},
		{"help",       no_argument,       NULL, 'h'},
		{NULL,         0,                 NULL,  0}
	};

	int i;

	pad_t pad[PADS_MAX];

	int passthrough  = 0;
	int buttons_only = 0;
	int numpads      = 1;
	verbosity        = 0;

	while((i = getopt_long(argc, argv, "p:v:nPVh", long_options, NULL)) != -1){
		switch(i) {
			case 'p':
				numpads = atoi(optarg);
				break;
			case 'v':
				verbosity = atoi(optarg);
				break;
			case 'n':
				buttons_only = 1;
				break;
			case 'P':
				passthrough = 1;
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

	if(numpads < 1 || numpads > PADS_MAX) {
		fprintf(stderr, "Only 1-%d gamepads can be used\n", PADS_MAX);
		return(EXIT_FAILURE);
	}

	printf("Initializing %d gamepad%s\n", 
	       numpads, 
	       numpads == 1 ? "" : "s");

	for(i = 0; i < numpads; i++) {

		memset(pad, 0, sizeof(pad_t));

		pad[i].num = 1 + i;
		pad[i].fd = uinput_init(pad[i].num, buttons_only);

		if(pad[i].fd < 0)
			return(pad[i].fd);

	}

	printf("Initializing serial interface\n");
	struct ftdi_context *ftdic = bub_init(57600, 1, 0, 0);

	if(ftdic == NULL)
		return(EXIT_FAILURE);

	ftdi_usb_purge_tx_buffer(ftdic);

	signal_install();

	busy = 1;

	while(busy) {

		for(i = 0; i < numpads; i++) {

			pad_t *p = &pad[i];

			ftdi_usb_purge_rx_buffer(ftdic);

			uint8_t req = i;

			bub_send(ftdic, &req, sizeof(req));
			bub_fetch(ftdic, &p->state, sizeof(req));

			if(p->state != p->last || passthrough) {
				uinput_map(p, buttons_only);
				p->last = p->state;
			}



		}

	}

	fprintf(stderr, "\nCaught signal %d\n", interrupt);

	printf("Closing serial interface\n");

	int ret = 0;
	if ((ret = ftdi_usb_close(ftdic)) < 0) {
		fprintf(stderr, "Unable to close ftdi device: %d (%s)\n", 
			ret,
		       	ftdi_get_error_string(ftdic));
		return(EXIT_FAILURE);
	}

	ftdi_deinit(ftdic);

	printf("Closing gamepad device%s\n", numpads == 1 ? "" : "s");

	for(i = 0; i < numpads; i++)
		uinput_deinit(&pad[i]);

	return(EXIT_SUCCESS);
}
