NES Gamepad Arduino Uinput Thing
--------------------------------

This repository contains two co-reliant programs.
Together they allow you to use up to 4 NES gamepads as standard joysticks
through a single USB port. On Linux.

You can also use the builtin Linux gamecon driver to use NES gamepads
via the RS232 serial port. This project is an alternative to that.

You can reach me via github: https://github.com/gammy

Components
----------

`uinput-nes`: A Linux userland driver which creates 1-4 standard joystick interfaces 
representing a real NES gamepad/controller. 
It relies on nes_arduino_relay (or something which does the same job) for 
requesting and receiving controller state data from a virtual serial 
interface.

`nes_arduino_relay`: An Arduino sketch which can decode NES gamepad data from up to 4 controllers.
It waits for a pad number request on the serial line and then sends that pad
state back.

`nes_test`: A barebone NES gamepad decoder which just sends human-readable data to the
serial port.

Requirements
------------
 * A USB port
 * Linux with uinput support (standard)
 * An Arduino Duemilanove or pin-compatible equivalent
 * The Arduino IDE (for programming the Arduino)
 * One or more NES gamepads

Build
-----

### Hardware ###

Connect the NES gamepads to your Arduino like this,

	Ground -> |1\_ 
	 Clock -> |2 5| <- +5V
	 Latch -> |3 6|
	  Data -> |4 7|
	
	        Gamepad | Arduino
	       ---------+-------
	       Ground 1 | Ground
	        Clock 2 | D2
	        Latch 3 | D3
	(Pad 1)  Data 4 | D4
	(Pad 2)  Data 4 | D5
	(Pad 3)  Data 4 | D6
	(Pad 4)  Data 4 | D7
	          +5V 5 | +5V

Then you need to connect the Arduino to your computer via the USB cable.
I presume that people who are reading this are mainly interested in using this code for their own purposes; the userland driver is fairly modular in that sense.

### Software ###

You'll need to build the software; both the uinput driver and the relay Arduino sketch, and for those things you of course need a build environment such as `build-essential` on debian/ubunututu or `base-devel` on archlinux, and the Arduino IDE of course. In addition, the uinput driver depends on `libftdi` for USB serial port communication.

 1. Clone this repository or download it somehow
 2. Go into `nes_gamepad/uinput-nes/` and build the program by running `make`, which will hopefully result in the production of a `uinput-nes` binary.
 3. Open the Arduino IDE and load the `nes_gamepad/nes_arduino_relay/` sketch
 4. Build and upload the sketch to your Arduino

That should be it.

### Usage ###

Just run `uinput-nes` as root.
By default it creates a single joystick interface (`js0` if no other joysticks are connected) in the most standard fashion. Run `uinput-nes --help` for a variety of options.

### Options ###
(I'm too lazy to document all of them, so I'll just mention the most interesting ones here; sorry!)

`--noaxis`

Sets up the D-pad (up, down, left, right) as standard buttons (1, 2, 3, 4) rather than axial devices.

`--passthrough`

uinput-nes will normally just pass on controller state changes to the uinput subsystem. For example, if A is pressed, that state change("A was pressed down") is sent. If the state of the controller remains the same, nothing is sent. In passthrough mode, all states read are immediately passed on rather than discarded. Some programs such as mednafen require the passthrough option as it expects key repetitions during a certain timeframe in order to set up the controllers. I've so far not come across any other programs requiring it.

`--daemon`

Makes uinput-nes a background process, which is appropriate if you want to have it running all the time. 

`--noftdi`

uinput-nes supports non-FTDI serial interfaces as seen on the newer Arduino boards using the Atmega 32u4 microprocessor (ie the `Arduino Leonardo`). Use this option if you have such a board.

