/* This file is part of uinput-nes.
 *
 * Copyright (C) 2013 Kristian Gunstone 
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
#include <getopt.h>
#include <libgen.h>

int verbosity = 0;
volatile int busy, interrupt;

void usage(char *me) {
	printf("uinput-nes v%s\n"
	       "Usage: %s [options]\n\n"
	       "-p  --pads <number>   Simulate <number> joypads           (default: 1, max: 4)\n"
	       "-v  --verbose <level> Verbosity level <level>             (default: 0, max: 2)\n"
	       "-n  --noaxis          Emulate D-pad with buttons                (default: off)\n"
	       "-P  --passthrough     Pass through data, not just state changes (default: off)\n"
	       "-d  --daemon          Become a daemon(background process)       (default: off)\n"
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
		{"daemon",     no_argument,       NULL, 'd'},
		{"version",    no_argument,       NULL, 'V'},
		{"help",       no_argument,       NULL, 'h'},
		{NULL,         0,                 NULL,  0}
	};

	int i;

	pad_t pad[PADS_MAX];

	int numpads      = 1;
	int buttons_only = 0;
	int passthrough  = 0;
	int daemonize    = 0;
	verbosity        = 0;

	while((i = getopt_long(argc, argv, "p:v:nPdVh", long_options, NULL)) != -1){
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
			case 'd':
				daemonize = 1;
				break;
			case 'h':
				usage(basename(argv[0]));
				return(EXIT_SUCCESS);
			case 'V':
				printf("uinput-nes v%s\n", VERSION);
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

		memset(&pad[i], 0, sizeof(pad_t));

		pad[i].num = 1 + i;
		pad[i].fd = uinput_init(pad[i].num, buttons_only);

		if(pad[i].fd < 0)
			return(pad[i].fd);

	}

	if(daemonize) {

		if(verbosity > 0)
			fprintf(stderr, "Becoming a background process\n");

		pid_t pid = fork();
		
		if(pid != 0) {
			if(verbosity > 1)
				fprintf(stderr, "Parent process exiting\n");
			return(EXIT_SUCCESS);
		} 
	}

	printf("Initializing serial interface\n");

	struct ftdi_context *ftdic = NULL;

	while(ftdic == NULL)
		ftdic = bub_connect();

	ftdi_usb_purge_tx_buffer(ftdic);

	signal_install();

	uint8_t rxbuf[2] = {0, 0};
	busy = 1;

	while(busy) {

		for(i = 0; i < numpads; i++) {

			if(ftdic != NULL)
				ftdi_usb_purge_rx_buffer(ftdic);

			uint8_t num = i;

			if(bub_send(ftdic, &num, sizeof(uint8_t)) < 0) {
				fprintf(stderr, "Serial send error\n");
				sleep(1);

				fprintf(stderr, "\nAttempting to connect\n");
				ftdic = bub_connect();

				break;
			}

			if(bub_fetch(ftdic, rxbuf,  sizeof(rxbuf)) < 0) {
				fprintf(stderr, "Serial fetch error\n");
				sleep(1);

				break;
			}

			num = rxbuf[0];

			pad_t *p = &pad[num];

			p->state = rxbuf[1];
			p->num   = num;

			if(p->num < 0 || p->num >= numpads)
				fprintf(stderr, "Received invalid pad number: %d\n", p->num);

			if(p->state != p->last || passthrough) {
				uinput_map(p, buttons_only);
				p->last = p->state;
			}

		}

	}

	if(verbosity > 0)
		fprintf(stderr, "\nCaught signal %d\n", interrupt);

	printf("Closing serial interface\n");
	bub_deinit(ftdic);

	printf("Closing gamepad device%s\n", numpads == 1 ? "" : "s");

	for(i = 0; i < numpads; i++)
		uinput_deinit(&pad[i]);

	return(EXIT_SUCCESS);
}
