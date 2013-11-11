#include "main.h"
#include "uinput.h"

int uinput_init(int device_number, int buttons_only) {

	int fd, i, r, numbuttons;
	struct uinput_user_dev uidev;

	char *button_name[] = {"A",     "B", "Start", "Select", 
	                       "Up", "Down",  "Left",  "Right"};

	int button[] = {BTN_A, BTN_B, BTN_START, BTN_SELECT, 
                        BTN_0, BTN_1,     BTN_2, BTN_3};
	int axis[]   = {ABS_X, ABS_Y};

	if(buttons_only)
		numbuttons = 8;
	else
		numbuttons = 4;

	if(device_number == 1) { // XXX not so elegant, but does the job
		for(i = 0; i < numbuttons; i++)
			printf("Button %d: %s\n", 1 + i, button_name[i]);
	}

	if(verbosity > 1)
		fprintf(stderr, "uinput_init(%d)\n", device_number);

	// XXX is /dev/input/uinput on some systems ?
	fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if(fd < 0) {
		perror("open");
		return(fd);
	}

	if(verbosity > 1)
		fprintf(stderr, "Gamepad %d (fd %d)\n", device_number, fd);

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
	for(i = 0; i < numbuttons; i++) {
		if(verbosity > 1) {
			fprintf(stderr, "Adding key %d type %03x (%s)\n", 
				1 + i, 
				button[i],
				button_name[i]);
		}

		r = ioctl(fd, UI_SET_KEYBIT, button[i]);
		if(r < 0) {
			perror("ioctl");
			return(r);
		}
	}

	if(! buttons_only) {
		// Add the axes
		for(i = 0; i < 2; i++) {

			if(verbosity > 1) {
				fprintf(stderr, "Adding axis %d, type %03x\n", 
					i, axis[i]);
			}

			r = ioctl(fd, UI_SET_ABSBIT, axis[i]);
			if(r < 0) {
				perror("ioctl");
				return(r);
			}

			// AXIS_MIN for left/up, AXIS_MAX for right/down
			uidev.absmin[axis[i]] = AXIS_MIN;
			uidev.absmax[axis[i]] = AXIS_MAX;

			uidev.absfuzz[axis[i]] = 0;
		}
	}

	// Initialize the device
	memset(&uidev, 0, sizeof(uidev));
	snprintf(uidev.name, 
		 UINPUT_MAX_NAME_SIZE, 
		 "NES gamepad %d", device_number);
	uidev.id.bustype = BUS_USB;
	uidev.id.vendor  = 0x1; // FIXME
	uidev.id.product = 0x2; // GC_NES in the gamecon driver (irrelevant)
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

void uinput_deinit(pad_t *pad) {

	if(verbosity > 1)
		fprintf(stderr, "Closing pad %d (fd %d)\n", pad->num, pad->fd);

	if(ioctl(pad->fd, UI_DEV_DESTROY) < 0)
		perror("ioctl");

	close(pad->fd);

}

int uinput_send(int fd, uint16_t type, uint16_t code, int32_t val){
	
	struct input_event ev;

	memset(&ev, 0, sizeof(struct input_event));

	ev.type = type;
	ev.code = code;
	ev.value = val;

	if(verbosity > 1) {
		printf("Sending event type %5d, code %5d, val %5d\n", 
		       type, code, val);
	}

	int r = write(fd, &ev, sizeof(struct input_event));
	if(r < 0)
		perror("write");

	return(r);
}

void printbits(uint8_t b) {
	int8_t i;

	for(i = 7; i >= 0; i--) 
		printf("%d", (b & (1 << i)) >> i);

	return;
}

void uinput_map(pad_t *pad, int buttons_only) {

	static unsigned long count = 0;

	uint8_t state = pad->state;
	int        fd = pad->fd;

	if(verbosity > 0) {

		printf("%8ld Pad %d: ", count, pad->num);
		printf("%3d (%2x) [", state, state);
		printbits(state);

		printf("] Right(%d)",   IS_RIGHT(state));
		printf(" Left(%d)",      IS_LEFT(state));
		printf(" Down(%d)",      IS_DOWN(state));
		printf(" Up(%d)",          IS_UP(state));
		printf(" Start(%d)",    IS_START(state));
		printf(" Select(%d)",  IS_SELECT(state));
		printf(" B(%d)",            IS_B(state));
		printf(" A(%d)",            IS_A(state));
		puts("");

		count++;
	}

	if(buttons_only) {
		uinput_send(fd, EV_KEY, BTN_0,    IS_UP(state));
		uinput_send(fd, EV_KEY, BTN_1,  IS_DOWN(state));
		uinput_send(fd, EV_KEY, BTN_2,  IS_LEFT(state));
		uinput_send(fd, EV_KEY, BTN_3, IS_RIGHT(state));
	} else {
		if(IS_UP(state))
			uinput_send(fd, EV_ABS, ABS_Y, AXIS_MIN);
		else if(IS_DOWN(state))
			uinput_send(fd, EV_ABS, ABS_Y, AXIS_MAX);
		else
			uinput_send(fd, EV_ABS, ABS_Y,  0);

		if(IS_LEFT(state))
			uinput_send(fd, EV_ABS, ABS_X, AXIS_MIN);
		else if(IS_RIGHT(state))
			uinput_send(fd, EV_ABS, ABS_X, AXIS_MAX);
		else
			uinput_send(fd, EV_ABS, ABS_X,  0);
	}

	uinput_send(fd, EV_KEY,  BTN_START,  IS_START(state));
	uinput_send(fd, EV_KEY, BTN_SELECT, IS_SELECT(state));
	uinput_send(fd, EV_KEY,      BTN_A,      IS_A(state));
	uinput_send(fd, EV_KEY,      BTN_B,      IS_B(state));

	uinput_send(fd, EV_SYN, SYN_REPORT, 0);

	return;
}
