/* Dual NES gamepad read/relay code on an Atmega328p microcontroller.
 * By gammy
 * 
 * Unlike nes_test.ino which prints human-readable information to the serial
 * port, this code waits for a pad # read request on the serial line from the
 * uinput driver, and then sends the state of that joypad as a byte.
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

#define SET_LED_LO		(PORTB &= ~(1 << 5))
#define SET_LED_HI		(PORTB |=  (1 << 5))

//#define DEBOUNCE_US             100000 // 100ms
#define DEBOUNCE_US             500 // 0.5ms

// Pad struct used for debouncing
typedef struct {
	uint32_t timer;
	uint8_t current;
	uint8_t last;
	uint8_t debounced;
} pad_t;
	
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

		data = REG_IN;

		pads[0] = (pads[0] << 1) | DATA_BIT(data, 0);
		pads[1] = (pads[1] << 1) | DATA_BIT(data, 1);
		pads[2] = (pads[2] << 1) | DATA_BIT(data, 2);
		pads[3] = (pads[3] << 1) | DATA_BIT(data, 3);

	}
}

uint8_t nes_read_debounced(uint8_t num) {

	static uint8_t current;
	static uint32_t us_last;

	uint8_t state[4];
	uint8_t read_new;

	nes_read(state);
	read_new = state[num];

	if(read_new != current)
		us_last = micros();

	if((micros() - us_last) > (uint32_t) DEBOUNCE_US)
		current = read_new;

	return(current);

}

void setup(void) {

	// Initialize outputs
	// Default pin mode is input, so we don't set those
	REG_DIRECTION |= (1 << PIN_LATCH) | 
                         (1 << PIN_CLOCK);

	// LED indicator ;p
	DDRB |= (1 << 5);

	// Enable pull-ups on inputs
	REG_OUT |= (1 << PIN_DATA_BASE + 0) | 
	           (1 << PIN_DATA_BASE + 1) |
                   (1 << PIN_DATA_BASE + 2) |
                   (1 << PIN_DATA_BASE + 3);

        Serial.begin(57600);
}

void loop(void) {

	uint8_t state[4];
	static pad_t pad[4];

	// Read all gamepad states
	nes_read(state);

	// Debounce 
	uint8_t i;
	for(i = 0; i < 4; i++) {

		pad_t *p = &pad[i];
		p->current = state[i];

		if(p->current != p->last)
			p->timer = micros();

		if((micros() - p->timer) > DEBOUNCE_US)
			p->debounced = p->current;

		p->last = p->current;
	}

	// Receive request & transmit state 
	while(Serial.available() > 0) {
		SET_LED_HI;

		uint8_t num = Serial.read();

		if(num >= 0 && num <= 3) {
			Serial.write(pad[num].debounced);
		}

		SET_LED_LO;
	}
}
