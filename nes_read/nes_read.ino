/* Example Dual NES gamepad read code on an Atmega328p microcontroller.
 * By gammy
 *
 * XXX This code(macros, init, read) assume that all 4 pins are on PORT D.
 *     Remember to adjust PORTD/PIND below if you're using anything but D0-D7.
 */

/** Globals ****************************************************************/
#define DEBUG_SERIAL

#ifdef DEBUG_SERIAL
#define SPRINT(x) 		Serial.print(x)
#define SPRINTLN(x)		Serial.println(x)
#else
#define SPRINT(x) 		(void) 0
#define SPRINTLN(a)		(void) 0
#endif

#define IS_RIGHT(x)		(x & 0b00000001)
#define IS_LEFT(x)		(x & 0b00000010)
#define IS_DOWN(x)		(x & 0b00000100)
#define IS_UP(x)		(x & 0b00001000)
#define IS_START(x)		(x & 0b00010000)
#define IS_SELECT(x)		(x & 0b00100000)
#define IS_B(x)			(x & 0b01000000)
#define IS_A(x)			(x & 0b10000000)

#define OUT_PORTD_CLOCK         2 // D2
#define OUT_PORTD_LATCH         3 // D3

#define IN_PORTD_DATA_2         4 // D4
#define IN_PORTD_DATA_1         5 // D5

#define SET_LATCH_LO		(PORTD &= ~(1 << OUT_PORTD_LATCH))
#define SET_LATCH_HI		(PORTD |=  (1 << OUT_PORTD_LATCH))

#define SET_CLOCK_LO		(PORTD &= ~(1 << OUT_PORTD_CLOCK)) 
#define SET_CLOCK_HI		(PORTD |=  (1 << OUT_PORTD_CLOCK))

#define DATA_1(x)		((~x >> IN_PORTD_DATA_1) & 0b00000001)
#define DATA_2(x)		((~x >> IN_PORTD_DATA_2) & 0b00000001)

/** Routines ***************************************************************/

void nes_read(uint8_t *pads) {

	uint8_t i;
	uint8_t data;

	pads[0] = pads[1] = 0;

	SET_CLOCK_HI;
	SET_LATCH_HI;
	SET_LATCH_LO;

	for(i = 0; i < 8; i++) {

		SET_CLOCK_LO;
		SET_CLOCK_HI;

		data = PIND;

		pads[0] = (pads[0] << 1) | DATA_1(data);
		pads[1] = (pads[1] << 1) | DATA_2(data);

	}
}

void setup(void) {

	// Initialize outputs
	// Default pin mode is input, so we don't set those
	DDRD |= (1 << OUT_PORTD_LATCH) | 
                (1 << OUT_PORTD_CLOCK);

	// Enable pull-ups on inputs
	PORTD |= (1 << IN_PORTD_DATA_1) | 
                 (1 << IN_PORTD_DATA_2);

#ifdef DEBUG_SERIAL
        Serial.begin(57600);
	SPRINTLN("NES Gamepad Reader");
#endif	

}

void loop(void) {

	uint8_t pads[2];
	uint8_t i;

	nes_read(pads);

	for(i = 0; i < 2; i++) {

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
