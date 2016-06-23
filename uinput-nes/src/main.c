/* This file is part of uinput-nes.
 *
 * Copyright (C) 2013, 2014 Kristian Gunstone 
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
#include "kbd.h"
#include <getopt.h>
#include <libgen.h>

int verbosity = 0;
volatile int busy, interrupt;

void usage(char *me) {
            
             //|------------------------------------------------------------------------------|
    printf("uinput-nes v%s\n"
           "Usage: %s [options]\n\n"
           "-p  --pads <number>   Simulate <number> joypads           (default: 1, max: 4)\n"
           "-v  --verbose <level> Verbosity level <level>             (default: 0, max: 2)\n"
           "-n  --noaxis          Emulate D-pad with buttons                (default: off)\n"
           "-N  --ftdi            Use FTDI interface (older boards)         (default: off)\n"
           "-P  --passthrough     Pass through data, not just state changes (default: off)\n"
           "-k  --keyboard [keys] Simulate key presses                      (default: \"default\")\n"
           "-d  --daemon          Become a daemon(background process)       (default: off)\n"
           "-D  --hwid <vid:pid>  Use <vid:pid> as vendor/product id  (default: 0403:6001)\n"
           "-s  --serialdev <dev> Use <dev> as serial device       (default: /dev/ttyACM0)\n"
           "-h  --help            This help\n"
           "-V  --version         Display version\n\n"
           "This is free software; see the source for copying conditions. "
           "There is NO\nwarranty; not even for MERCHANTABILITY or FITNESS "
           "FOR A PARTICULAR PURPOSE.\n\n",
           VERSION, me);
    kbd_print_keys();
}

int main(int argc, char *argv[]) {

    static struct option const long_options[] = {
        {"pads",       required_argument, NULL, 'p'},
        {"verbose",    required_argument, NULL, 'v'},
        {"noaxis",     no_argument,       NULL, 'n'},
        {"ftdi",       no_argument,       NULL, 'f'},
        {"passthrough",no_argument,       NULL, 'P'},
        {"keyboard",   required_argument, NULL, 'k'},
        {"daemon",     no_argument,       NULL, 'd'},
        {"hwid",       required_argument, NULL, 'D'},
        {"serialdev",  required_argument, NULL, 's'},
        {"version",    no_argument,       NULL, 'V'},
        {"help",       no_argument,       NULL, 'h'},
        {NULL,         0,                 NULL,  0}
    };

    const char *short_options = "p:v:nfPk:dD:s:Vh";

    int i;

    int numpads      = 1;
    pad_t pad[PADS_MAX];
    kbdopt_parse("a1:Space,"
                 "b1:LeftCTRL,"
                 "start1:Enter,"
                 "select1:E,"
                 "up1:Up,"
                 "down1:Down,"
                 "left1:Left,"
                 "right1:Right,"
                 // Player 2 (super uncomfortable)
                 "a2:P,"
                 "b2:O,"
                 "start2:0,"
                 "select2:9,"
                 "up2:Home,"
                 "down2:End,"
                 "left2:Delete,"
                 "right2:PageDown", 
                 pad, 2);

    int emulation_mode = UINPUT_MODE_JOYSTICK;
    int passthrough  = 0;
    int daemonize    = 0;

    int iface_type   = BUB_TYPE_SERIAL;

    char *serial_dev = SERIAL_PORT;
    char *tmp_vendor = NULL;
    int usb_vendor   = 0x0403;
    int usb_product  = 0x6001;
    verbosity        = 0;

    while((i = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
        switch(i) {
            case 'p':
                numpads = atoi(optarg);
                break;
            case 'v':
                verbosity = atoi(optarg);
                break;
            case 'n':
                emulation_mode = UINPUT_MODE_JOYSTICK_NO_AXIS;
                break;
            case 'f':
                iface_type = BUB_TYPE_FTDI;
                break;
            case 'P':
                passthrough = 1;
                break;
            case 'k':
                emulation_mode = UINPUT_MODE_KEYBOARD;
                if(strncasecmp(optarg, "default", strlen("default")) != 0) {
                    if(kbdopt_parse(optarg, pad, numpads ) != 0) { // FIXME numpads must precede this argument :/
                        usage(basename(argv[0]));
                        return(EXIT_FAILURE);
                    }
                }
                break;
            case 'd':
                daemonize = 1;
                break;
            case 'D':
                tmp_vendor = strsep(&optarg, ":");
                if(optarg == NULL) {
                    fprintf(stderr, "Invalid hardware id\n");
                    usage(basename(argv[0]));
                    return(EXIT_FAILURE);
                }
                usb_vendor  = strtol(tmp_vendor, NULL, 16);
                usb_product = strtol(optarg, NULL, 16);
                break;
            case 's':
                serial_dev = optarg;

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

    bub_t *bub = bub_init(iface_type);

    bub->ftdi_vid      = usb_vendor;
    bub->ftdi_pid      = usb_product;
    bub->serial_dev    = serial_dev;
    bub->baud_rate     = SERIAL_BAUDRATE;

    busy = 1;
    signal_install();

    printf("Initializing %d gamepad%s\n", 
           numpads, 
           numpads == 1 ? "" : "s");

    for(i = 0; i < numpads; i++) {

        //memset(&pad[i], 0, sizeof(pad_t));

        pad[i].num = 1 + i;
        pad[i].state = pad[i].last = 0;

        if(uinput_init(&pad[i], emulation_mode) != 0) {
            return(EXIT_FAILURE);
        }
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

    while(busy) {

        if(bub_connect(bub) != 0)
            sleep(1);
        else
            break;

    }

    bub_flush(bub);

    uint8_t rxbuf[2] = {0, 0};

    while(busy) {

        for(i = 0; i < numpads; i++) {

            bub_iflush(bub);

            uint8_t num = i;

            if(bub_send(bub, &num, sizeof(uint8_t)) != sizeof(uint8_t)) {
                fprintf(stderr, "Serial send error\n");
                sleep(1);

                bub_deinit(bub);
                fprintf(stderr, "\nAttempting to connect\n");
                bub_connect(bub);

                break;
            }

            if(bub_fetch(bub, rxbuf,  sizeof(rxbuf)) < 0) {
                fprintf(stderr, "Serial fetch error\n");
                sleep(1);

                break;
            }

            num = rxbuf[0];

            if(num < 0 || num >= numpads) {
                fprintf(stderr, "Received invalid pad number: %d\n", num);
                break;
            }

            pad_t *p = &pad[num];

            p->state = ~rxbuf[1];
            p->num   = num;

            if(p->state != p->last || passthrough) {
                uinput_map(p, emulation_mode);
                p->last = p->state;
            }

        }

    }

    if(verbosity > 0)
        fprintf(stderr, "\nCaught signal %d\n", interrupt);

    printf("Closing serial interface\n");
    fflush(stdout);

    bub_deinit(bub);

    printf("Closing gamepad device%s\n", numpads == 1 ? "" : "s");

    for(i = 0; i < numpads; i++)
        uinput_deinit(&pad[i]);

    return(EXIT_SUCCESS);
}
