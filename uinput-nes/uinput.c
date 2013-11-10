#include "main.h"
#include "uinput.h"

int uinput_init(int device_number) {

	int fd, i, r;
	struct uinput_user_dev uidev;

#ifndef UINPUT_NOAXIS
	int button[] = {BTN_A, BTN_B, BTN_START, BTN_SELECT};
	int axis[]   = {ABS_X, ABS_Y};
#else
	int button[] = {BTN_A, BTN_B, BTN_START, BTN_SELECT, 
                        BTN_0, BTN_1, BTN_2,     BTN_3};
#endif

	if(verbosity > 1)
		fprintf(stderr, "uinput_init(%d)\n", device_number);

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

#ifndef UINPUT_NOAXIS
	// Add the axes
	for(i = 0; i < sizeof(axis) / sizeof(int); i++) {
		r = ioctl(fd, UI_SET_ABSBIT, axis[i]);
		if(r < 0) {
			perror("ioctl");
			return(r);
		}

		// -1 for left/up, 1 for right/down
		uidev.absmin[axis[i]]  = -1;
		uidev.absmax[axis[i]]  =  1;

		uidev.absfuzz[axis[i]] = 0;
	}
#endif

	// Initialize the device
	memset(&uidev, 0, sizeof(uidev));
	snprintf(uidev.name, 
		 UINPUT_MAX_NAME_SIZE, 
		 "NES gamepad (%d)", device_number);
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

	if(verbosity > 1)
		fprintf(stderr, "Device %d fd = %d\n", device_number, fd);

	return(fd);

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

void uinput_map_buttons(int fd, uint8_t state) {

	static unsigned long count = 0;

	if(verbosity > 0) {
		printf("%8ld: Pad fd %d: ", count, fd);
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
	}

#ifndef UINPUT_NOAXIS
	// FIXME should this be ABS_{X,Y}, not REL?
	// Seems like it:
	// https://github.com/torvalds/linux/blob/master/drivers/input/joystick/turbografx.c#L106
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
#else
	uinput_send(fd, EV_KEY, BTN_0,    IS_UP(state));
	uinput_send(fd, EV_KEY, BTN_1,  IS_DOWN(state));
	uinput_send(fd, EV_KEY, BTN_2,  IS_LEFT(state));
	uinput_send(fd, EV_KEY, BTN_3, IS_RIGHT(state));
#endif

	uinput_send(fd, EV_KEY,  BTN_START,  IS_START(state));
	uinput_send(fd, EV_KEY, BTN_SELECT, IS_SELECT(state));
	uinput_send(fd, EV_KEY,      BTN_A,      IS_A(state));
	uinput_send(fd, EV_KEY,      BTN_B,      IS_B(state));

	uinput_send(fd, EV_SYN, SYN_REPORT, 0);

	return;
}
