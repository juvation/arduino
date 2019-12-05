// NRPN.ino

// the Prophet VS NRPN reverser!
// allowing any NRPN editor to edit VS parameters

// INCLUDES

#include <MIDI.h>

// MIDI MESSAGES

#define MIDI_PARAMETER_SELECT_LSB	0x62
#define MIDI_PARAMETER_SELECT_MSB	0x63

#define MIDI_PARAMETER_VALUE_LSB	0x26
#define MIDI_PARAMETER_VALUE_MSB	0x06

// DIGITAL PIN ASSIGNMENTS

#define BUTTON_1  2
#define BUTTON_2  3
#define BUTTON_3  4

#define LED_GREEN 6
#define LED_RED 7

#define LED 13           // LED pin on Arduino Uno

// ANALOGUE PIN ASSIGNMENTS

#define KNOB_1  A0
#define KNOB_2  A1

// TYPEDEFS

// PROTOTYPES

void
onControlChange (byte inChannel, byte inControlNumber, byte inValue);

// GLOBAL DATA

MIDI_CREATE_DEFAULT_INSTANCE();

byte	gParameterSelectMSB = 0;
byte	gParameterValueMSB = 0;

// SETUP

void
setup ()
{
  pinMode (BUTTON_1, INPUT_PULLUP);
  pinMode (BUTTON_2, INPUT_PULLUP);
  pinMode (BUTTON_3, INPUT_PULLUP);

  pinMode (LED_GREEN, OUTPUT);
  pinMode (LED_RED, OUTPUT);
  
  // LEDs are active low for some dumb reason
  digitalWrite (LED_GREEN, HIGH);
  digitalWrite (LED_RED, HIGH);

	MIDI.setHandleControlChange (onControlChange);
	
  // Launch MIDI and listen to channel 1
  MIDI.begin (1);
}

// LOOP

void
loop ()
{
}

// MIDI PACKET HANDLERS

// the logic goes that conventional NRPNs send both parameter selects
// when the new parameter number is >127
// but only the LSB when <=127
// so when we receive an MSB, we store it
// and when we receive an LSB, we send a reversed parameter select to the VS
// same with the value messages
void
onControlChange (byte inChannel, byte inControlNumber, byte inValue)
{
	if (inControlNumber == MIDI_PARAMETER_SELECT_LSB)
	{
    MIDI.sendControlChange (inChannel, MIDI_PARAMETER_SELECT_LSB, inValue);
    MIDI.sendControlChange (inChannel, MIDI_PARAMETER_SELECT_MSB, gParameterSelectMSB);
	}
	else
	if (inControlNumber == MIDI_PARAMETER_SELECT_MSB)
	{
		gParameterSelectMSB = inValue;
	}
	else
	if (inControlNumber == MIDI_PARAMETER_VALUE_LSB)
	{
    MIDI.sendControlChange (inChannel, MIDI_PARAMETER_VALUE_LSB, inValue);
    MIDI.sendControlChange (inChannel, MIDI_PARAMETER_VALUE_MSB, gParameterValueMSB);
	}
	else
	if (inControlNumber == MIDI_PARAMETER_VALUE_MSB)
	{
		gParameterValueMSB = inValue;
	}
	
}

