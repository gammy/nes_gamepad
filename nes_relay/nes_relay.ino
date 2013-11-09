/* Dual NES gamepad read/relay code on an Atmega328p microcontroller.
 * By gammy
 *
 * XXX This code(macros, init, read) assume that all 4 pins are on PORT D.
 *     Remember to adjust PORTD/PIND below if you're using anything but D0-D7.
 * 
 * Unlike nes_read.ino which is an example, this program deserializes the 
 * NES keypads and sends each gamepad as a single byte on the serial line.
 * IE, it takes the 16 individual bits from the controllers and sends them
 * as 2 bytes.
 */

/** Globals ****************************************************************/
#ifdef DEBUG_SERIAL
#define SPRINT(x) 		Serial.print(x)
#define SPRINTLN(x)		Serial.println(x)
#else
#define SPRINT(x) 		(void) 0
#define SPRINTLN(a)		(void) 0
#endif

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

        Serial.begin(57600);
}

void loop(void) {

	uint8_t pads[2];

	nes_read(pads);

	if(! Serial)
		return;

	Serial.write(pads, sizeof(pads));
	Serial.flush();

}
