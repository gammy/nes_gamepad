#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <ftdi.h>
#include <linux/input.h>
#include <linux/uinput.h>

#define IS_RIGHT(x)		(x & 0b00000001)
#define IS_LEFT(x)		(x & 0b00000010)
#define IS_DOWN(x)		(x & 0b00000100)
#define IS_UP(x)		(x & 0b00001000)
#define IS_START(x)		(x & 0b00010000)
#define IS_SELECT(x)		(x & 0b00100000)
#define IS_B(x)			(x & 0b01000000)
#define IS_A(x)			(x & 0b10000000)

volatile int busy, interrupt;

void signal_handle(int sig) {

	interrupt = sig;
        busy = 0;

}

void signal_install(void) {

        struct sigaction sig_act_old, sig_act_new;

        sig_act_new.sa_handler = signal_handle;
        sigemptyset(&sig_act_new.sa_mask);

        sig_act_new.sa_flags = 0;

        sigaction(SIGINT, NULL, &sig_act_old);

        if(sig_act_old.sa_handler != SIG_IGN)
                sigaction(SIGINT, &sig_act_new, NULL);

        sigaction(SIGHUP, NULL, &sig_act_old);

        if(sig_act_old.sa_handler != SIG_IGN)
                sigaction(SIGHUP, &sig_act_new, NULL);

        sigaction(SIGTERM, NULL, &sig_act_old);

        if(sig_act_old.sa_handler != SIG_IGN)
                sigaction (SIGTERM, &sig_act_new, NULL);

}


int uinput_init(void) {

	int fd, i, r;
	struct uinput_user_dev uidev;

	int button[] = {BTN_A, BTN_B, BTN_START, BTN_SELECT};
	int axis[]   = {ABS_X, ABS_Y};

	// XXX is /dev/input/uinput on some systems ?
	fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if(fd < 0) {
		perror("open");
		return(fd);
	}

	// Add Key(buttons) type
	r = ioctl(fd, UI_SET_EVBIT, EV_KEY);
	if(r < 0) { 
		perror("ioctl"); 
		return(r); 
	}

	// Add Absolute (D-pad) type
	r = ioctl(fd, UI_SET_EVBIT, EV_ABS);
	if(r < 0) { 
		perror("ioctl"); 
		return(r); 
	}

	// Add the buttons
	for(i = 0; i < sizeof(button) / sizeof(int); i++) {
		r = ioctl(fd, UI_SET_KEYBIT, button[i]);
		if(r < 0) {
			perror("ioctl");
			return(r);
		}
	}

	// Add the axes
	for(i = 0; i < sizeof(axis) / sizeof(int); i++) {
		r = ioctl(fd, UI_SET_ABSBIT, axis[i]);
		if(r < 0) {
			perror("ioctl");
			return(r);
		}

		// -1 for left/up, 1 for right/down
		uidev.absmin[axis[i]] = -1;
		uidev.absmax[axis[i]] =  1;
	}

	// Initialize the device
	memset(&uidev, 0, sizeof(uidev));
	snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "uinput-nes");
	uidev.id.bustype = BUS_USB;
	uidev.id.vendor  = 0x1; // FIXME
	uidev.id.product = 0x1; // FIXME
	uidev.id.version = 1;

	r = write(fd, &uidev, sizeof(uidev));
	if(r < 0) {
		perror("write");
		return(r);
	}

	r = ioctl(fd, UI_DEV_CREATE);
	if(r < 0) {
		perror("ioctl");
		return(r);
	}

	return(fd);

}

int uinput_send(int fd, uint16_t type, uint16_t code, int32_t val){
	
	struct input_event ev;

	memset(&ev, 0, sizeof(struct input_event));

	ev.type = type;
	ev.code = code;
	ev.value = val;

	printf("Sending event type %5d, code %5d, val %5d\n", 
	       type, code, val);
	int r = write(fd, &ev, sizeof(struct input_event));
	if(r < 0)
		perror("write");

	return(r);
}

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

int main(int argc, char *argv[]) {

	int i;
	uint8_t pads[2];
	const size_t bufsize = 2; //sizeof(pads);

	printf("Pad size(bytes): %ldb\n", sizeof(pads));

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
