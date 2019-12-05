// CVToMIDI.ino

// or... the Prophet VS hardware controller project!


#include <MIDI.h>

// Simple tutorial on how to receive and send MIDI messages.
// Here, when receiving any message on channel 4, the Arduino
// will blink a led and play back a note for 1 second.

MIDI_CREATE_DEFAULT_INSTANCE();

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

#define CV_INPUT	A2
#define GATE_INPUT	?

// GLOBALS

int	gGate = 0;
int	gNote = 0;

// SETUP

void
setup ()
{
	// REGULAR ARD MIDI SETUP
	
  pinMode (BUTTON_1, INPUT_PULLUP);
  pinMode (BUTTON_2, INPUT_PULLUP);
  pinMode (BUTTON_3, INPUT_PULLUP);

  pinMode (LED_GREEN, OUTPUT);
  pinMode (LED_RED, OUTPUT);
  
  // LEDs are active high for some dumb reason
  digitalWrite (LED_GREEN, HIGH);
  digitalWrite (LED_RED, HIGH);
  
  // OUR EXTRA BITS
  
	pinMode (GATE_INPUT, INPUT_PULLDOWN);
	  
  // Launch MIDI and listen to channel 1
  MIDI.begin (1);
}

// LOOP

void
loop ()
{
	int	gate = digitalRead (GATE_INPUT);
	
	if (gate)
	{
		if (gGate)
		{
			// gate still up
		}
		else
		{
			// new gate
			gCV = analogRead (CV_INPUT);
			
			sendMIDINoteOn ();
		}
	}
	else
	{
		if (gGate)
		{
			sendMIDINoteOff ();
		}
		else
		{
			int	note = analogRead (CV_INPUT);
			note /= 60;
			note += 34;
			
			if (gNote != note)
			{
				sendMIDINoteOff ();
				gNote = note;
				sendMIDINoteOn ();
			}
		}
	}
	
	gGate = gate;
}

void
sendMIDINoteOn ()
{
	MIDI.sendNoteOn (0, gNote, 64);
}

void
sendMIDINoteOff ()
{
	MIDI.sendNoteOn (0, gNote, 0);
}
