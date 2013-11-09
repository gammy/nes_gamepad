#include "main.h"
#include "uinput.h"

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

#ifdef DEBUG_UINPUT_MORE
	printf("Sending event type %5d, code %5d, val %5d\n", 
	       type, code, val);
#endif
	int r = write(fd, &ev, sizeof(struct input_event));
	if(r < 0)
		perror("write");

	return(r);
}

#ifdef DEBUG_UINPUT
void printbits(uint8_t b) {
	int8_t i;

	for(i = 7; i >= 0; i--) 
		printf("%d", (b & (1 << i)) >> i);

	return;
}
#endif

void uinput_map_buttons(int fd, uint8_t state) {

#ifdef DEBUG_UINPUT
	static unsigned long count = 0;
#endif


#ifdef DEBUG_UINPUT
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

	return;
}
