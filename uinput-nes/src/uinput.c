/* This file is part of uinput-nes.
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
#include "uinput.h"

int uinput_init(pad_t *pad, int mode) {

    int fd, i, r, numbuttons;
    struct uinput_user_dev uidev;

    char *button_name[] = {"A",     "B", "Start", "Select", 
                           "Up", "Down",  "Left",  "Right"};

    int button[] = {
        BTN_A, 
        BTN_B, 
        BTN_START, 
        BTN_SELECT, 
        BTN_0, 
        BTN_1,     
        BTN_2, 
        BTN_3
    };

    int *keyptr = NULL;

    int axis[]   = {ABS_X, ABS_Y};

    switch(mode) {
        default:
        case UINPUT_MODE_JOYSTICK:
            numbuttons = 4;
            keyptr = button;
            break;
        case UINPUT_MODE_JOYSTICK_NO_AXIS:
            numbuttons = 8;
            keyptr = button;
            break;
        case UINPUT_MODE_KEYBOARD:
            numbuttons = 8;
            keyptr = pad->kbdsym;
            break;
    }

    if(verbosity > 1)
        fprintf(stderr, "uinput_init(%d)\n", pad->num);

    memset(&uidev, 0, sizeof(struct uinput_user_dev));

    // Setup gamepad properties
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "NES gamepad %d", pad->num);
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor  = 0x1; // FIXME
    uidev.id.product = 0x2; // GC_NES in the gamecon driver (irrelevant)
    uidev.id.version = 1;

    // XXX is /dev/input/uinput on some systems ?
    fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if(fd < 0) {
        fprintf(stderr, "Cannot open \"%s\" for write: %s\n",
            "/dev/uinput",
            strerror(errno));
        return(fd);
    }

    if(verbosity > 1)
        fprintf(stderr, "Gamepad %d (fd %d)\n", pad->num, fd);

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
                keyptr[i],
                button_name[i]);
        }

        r = ioctl(fd, UI_SET_KEYBIT, keyptr[i]);
        if(r < 0) {
            perror("ioctl");
            return(r);
        }
    }

    if(mode == UINPUT_MODE_JOYSTICK) {
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
    
    pad->fd = fd;

    return(0);
}

void uinput_deinit(pad_t *pad) {

    if(verbosity > 1)
        fprintf(stderr, "Closing pad %d (fd %d)\n", pad->num, pad->fd);

    if(ioctl(pad->fd, UI_DEV_DESTROY) < 0)
        perror("ioctl");

    close(pad->fd);

}

int uinput_send(pad_t *pad, uint16_t type, uint16_t code, int32_t val){
    
    struct input_event ev;

    memset(&ev, 0, sizeof(struct input_event));

    ev.type = type;
    ev.code = code;
    ev.value = val;

    if(verbosity > 1) {
        printf("Sending event type %5d, code %5d, val %5d\n", 
               type, code, val);
    }

    int r = write(pad->fd, &ev, sizeof(struct input_event));
    if(r < 0) {
        fprintf(stderr, "Cannot write pad %d(fd %d) event: %s\n", 
            pad->num, 
            pad->fd, 
            strerror(errno));
    }

    return(r);
}

void printbits(uint8_t b) {
    int8_t i;

    for(i = 7; i >= 0; i--) 
        printf("%d", (b & (1 << i)) >> i);

    return;
}

void uinput_map(pad_t *pad, int mode) {

    static unsigned long count = 0;

    uint8_t state = pad->state;

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

    switch(mode) {
        default:

        case UINPUT_MODE_JOYSTICK:
            if(IS_UP(state))
                uinput_send(pad, EV_ABS, ABS_Y, AXIS_MIN);
            else if(IS_DOWN(state))
                uinput_send(pad, EV_ABS, ABS_Y, AXIS_MAX);
            else
                uinput_send(pad, EV_ABS, ABS_Y,  0);

            if(IS_LEFT(state))
                uinput_send(pad, EV_ABS, ABS_X, AXIS_MIN);
            else if(IS_RIGHT(state))
                uinput_send(pad, EV_ABS, ABS_X, AXIS_MAX);
            else
                uinput_send(pad, EV_ABS, ABS_X,  0);
            break;

            uinput_send(pad, EV_KEY,  BTN_START,  IS_START(state));
            uinput_send(pad, EV_KEY, BTN_SELECT, IS_SELECT(state));
            uinput_send(pad, EV_KEY,      BTN_A,      IS_A(state));
            uinput_send(pad, EV_KEY,      BTN_B,      IS_B(state));

        case UINPUT_MODE_JOYSTICK_NO_AXIS:
            uinput_send(pad, EV_KEY, BTN_0,    IS_UP(state));
            uinput_send(pad, EV_KEY, BTN_1,  IS_DOWN(state));
            uinput_send(pad, EV_KEY, BTN_2,  IS_LEFT(state));
            uinput_send(pad, EV_KEY, BTN_3, IS_RIGHT(state));

            uinput_send(pad, EV_KEY,  BTN_START,  IS_START(state));
            uinput_send(pad, EV_KEY, BTN_SELECT, IS_SELECT(state));
            uinput_send(pad, EV_KEY,      BTN_A,      IS_A(state));
            uinput_send(pad, EV_KEY,      BTN_B,      IS_B(state));
            break;

        case UINPUT_MODE_KEYBOARD:
            uinput_send(pad, EV_KEY, pad->kbdsym[INDEX_UP],    IS_UP(state));
            uinput_send(pad, EV_KEY, pad->kbdsym[INDEX_DOWN],  IS_DOWN(state));
            uinput_send(pad, EV_KEY, pad->kbdsym[INDEX_LEFT],  IS_LEFT(state));
            uinput_send(pad, EV_KEY, pad->kbdsym[INDEX_RIGHT], IS_RIGHT(state));
            uinput_send(pad, EV_KEY, pad->kbdsym[INDEX_START],  IS_START(state));
            uinput_send(pad, EV_KEY, pad->kbdsym[INDEX_SELECT], IS_SELECT(state));
            uinput_send(pad, EV_KEY, pad->kbdsym[INDEX_A],      IS_A(state));
            uinput_send(pad, EV_KEY, pad->kbdsym[INDEX_B],      IS_B(state));
            break;
    }


    uinput_send(pad, EV_SYN, SYN_REPORT, 0);

    return;
}
