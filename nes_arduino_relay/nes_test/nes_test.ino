/* Example Dual NES gamepad read code on an Atmega328p microcontroller.
 *
 * Copyright (C) 2013 Kristian Gunstone
 * 
 * Unlike nes_test.ino which prints human-readable information to the serial
 * port, this code waits for a pad # read request on the serial line from the
 * uinput driver, and then sends the state of that joypad as a byte.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. 
 *
 */

/** Globals ****************************************************************/

#define DEBUG_SERIAL

#ifdef DEBUG_SERIAL
#define SPRINT(x)               Serial.print(x)
#define SPRINTLN(x)             Serial.println(x)
#else
#define SPRINT(x)               (void) 0
#define SPRINTLN(a)             (void) 0
#endif

#define REG_DIRECTION           DDRD  // Port D data direction register
#define REG_IN                  PIND  // Port D read register
#define REG_OUT                 PORTD // Port D r/w register

#define PIN_CLOCK               2     // D2 (if port D is used)
#define PIN_LATCH               3     // D3
#define PIN_DATA_BASE           4     // D4; first gamepad is here. 
                                      //     2nd on D5, 3rd on D6, 4th on D7.

#define SET_LATCH_LO            (REG_OUT &= ~(1 << PIN_LATCH))
#define SET_LATCH_HI            (REG_OUT |=  (1 << PIN_LATCH))

#define SET_CLOCK_LO            (REG_OUT &= ~(1 << PIN_CLOCK)) 
#define SET_CLOCK_HI            (REG_OUT |=  (1 << PIN_CLOCK))

#define DATA_BIT(x, y)          ((~x >> (PIN_DATA_BASE + y)) & 0b00000001)

#define IS_RIGHT(x)             ((x & (1 << 0)) >> 0)
#define IS_LEFT(x)              ((x & (1 << 1)) >> 1)
#define IS_DOWN(x)              ((x & (1 << 2)) >> 2)
#define IS_UP(x)                ((x & (1 << 3)) >> 3)
#define IS_START(x)             ((x & (1 << 4)) >> 4)
#define IS_SELECT(x)            ((x & (1 << 5)) >> 5)
#define IS_B(x)                 ((x & (1 << 6)) >> 6)
#define IS_A(x)                 ((x & (1 << 7)) >> 7)

/** Routines ***************************************************************/

void nes_read(uint8_t *pads) {

	uint8_t i;
	uint8_t data;

	pads[0] = pads[1] = pads[2] = pads[3] = 0;

	SET_CLOCK_HI;
	SET_LATCH_HI;
	SET_LATCH_LO;

	for(i = 0; i < 8; i++) {

		SET_CLOCK_LO;
		SET_CLOCK_HI;

		data = REG_IN;

		pads[0] = (pads[0] << 1) | DATA_BIT(data, 0);
		pads[1] = (pads[1] << 1) | DATA_BIT(data, 1);
		pads[2] = (pads[2] << 1) | DATA_BIT(data, 2);
		pads[3] = (pads[3] << 1) | DATA_BIT(data, 3);

	}
}

void setup(void) {

	// Initialize outputs
	// Default pin mode is input, so we don't set those
	REG_DIRECTION |= (1 << PIN_LATCH) | 
                         (1 << PIN_CLOCK);

	// Enable pull-ups on inputs
	REG_OUT |= (1 << PIN_DATA_BASE + 0) | 
	           (1 << PIN_DATA_BASE + 1) |
                   (1 << PIN_DATA_BASE + 2) |
                   (1 << PIN_DATA_BASE + 3);

        Serial.begin(57600);
	SPRINTLN("NES Gamepad Reader");

}

void loop(void) {

	uint8_t pads[4];
	uint8_t i;

	nes_read(pads);

	for(i = 0; i < 4; i++) {

		uint8_t pad = pads[i];

		if(pad == 0)
			continue;

		SPRINT("Pad "); SPRINT(i); SPRINT(" ");

		SPRINT((pad & 0b00000001) >> 0);
		SPRINT((pad & 0b00000010) >> 1);
		SPRINT((pad & 0b00000100) >> 2);
		SPRINT((pad & 0b00001000) >> 3);
		SPRINT((pad & 0b00010000) >> 4);
		SPRINT((pad & 0b00100000) >> 5);
		SPRINT((pad & 0b01000000) >> 6);
		SPRINT((pad & 0b10000000) >> 7);
		SPRINT(": ");

		if(IS_UP(pad))
		   SPRINT("Up ");
		if(IS_DOWN(pad))
		   SPRINT("Down ");
		if(IS_LEFT(pad))
		   SPRINT("Left ");
		if(IS_RIGHT(pad))
		   SPRINT("Right ");
		if(IS_START(pad))
		   SPRINT("Start ");
		if(IS_SELECT(pad))
		   SPRINT("Select ");
		if(IS_A(pad))
		   SPRINT("A ");
		if(IS_B(pad))
		   SPRINT("B ");

		SPRINTLN("");
	}

}
