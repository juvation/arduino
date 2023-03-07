/*
	MIDIToSync24.fpe
	
	MIDI to Sync24 converter
	by Jason Proctor

	inputs -
	digital pin 0 (RX) - MIDI via optocoupler
	analogue pin 2 - clock divider (TODO)
	analogue pin 3 - sync24 clock pulse width (ms) (TODO)

	outputs -	
	digital pin 2 - sync24 start
	digital pin 3 - sync24 clock
	
*/

// INCLUDES

#include "MIDI.h"

// FLAGS

#define DIVISION

// EQUATES

// I/O pins

// analogue pins
#define	kDivisionPotPin 2

// digital pins
#define	kStartPin 2
#define kStopPin 3
#define	kClockPin 4
#define kStartLEDPin 5
#define kClockLEDPin 6

// PROTOTYPES

void
onMIDIClock ();

void
onMIDIContinue ();

void
onMIDIStart ();

void
onMIDIStop ();

// GLOBALS

boolean
gClockRunning = false;

boolean
gShuffle = false;

// this doesn't need to be big, it rolls over on a division boundary
int
gClockCount = 0;

// this many clocks gives a pulse on the clock output pin
int
gClockDivision = 0;

// division values to pot measurements
int
gClockDivisors [] = 
{
	1,	// 24ppqn
	3,	// 1/32nd note
	6,	// 1/16th note
	12,	// 1/8th note
	24,	// 1/4 note
	48,	// 1/2 note
	96	// whole note
};

#define kNumClockDivisors (sizeof (gClockDivisors) / sizeof (gClockDivisors [0]))

// MIDI library interface
MIDIClient	gMIDIClient;
MIDI	gMIDI;

// SETUP

void
setup ()
{
	// digital
	pinMode (kStartPin, OUTPUT);
        pinMode (kStopPin, OUTPUT);
	pinMode (kClockPin, OUTPUT);
	pinMode (kStartLEDPin, OUTPUT);
	pinMode (kClockLEDPin, OUTPUT);
        // Set port B pins 0..2 to output
        DDRB = DDRB | B00000111;
	
	// reset the gate and clock outputs
	digitalWrite (kStartPin, LOW);
        digitalWrite (kStopPin, LOW);
	digitalWrite (kClockPin, LOW);
	
	setClockDivision (analogRead (kDivisionPotPin));

	// MIDI init
	
	gMIDIClient.onClock = onMIDIClock;
	gMIDIClient.onContinue = onMIDIContinue;
	gMIDIClient.onStart = onMIDIStart;
	gMIDIClient.onStop = onMIDIStop;

	gMIDI.begin (&gMIDIClient);
}

// LOOP

void
loop ()
{
	gMIDI.poll ();

	setClockDivision (analogRead (kDivisionPotPin));
}

void
setClockDivision (int inPotValue)
{
	// scale the pot value into an index into the divisor table
	int	divider = 1024 / (kNumClockDivisors - 1);
	int	index = inPotValue / divider;

	gClockDivision = gClockDivisors [index];
        PORTB = index;
}

// MIDI EVENT HANDLERS

void
onMIDIClock ()
{
	if (gClockRunning)
	{
		// flip shuffle every 16th
		if ((gClockCount % 6) == 0)
		{
			gShuffle = !gShuffle;
		}
		
		if (gShuffle)
		{
			delay (10);
		}

#ifdef DIVISION
		if ((gClockCount % gClockDivision) == 0)
		{
#endif
			digitalWrite (kClockPin, HIGH);
			digitalWrite (kClockLEDPin, HIGH);
			delay (5);
			digitalWrite (kClockPin, LOW);
			digitalWrite (kClockLEDPin, LOW);
                        digitalWrite (kStartPin, LOW);

			// rollover on the bar line
			if ((gClockCount % 96) == 0)
			{
				gClockCount = 0;
			}

#ifdef DIVISION
		}
#endif

		gClockCount++;
	}
}

void
onMIDIContinue ()
{
	gClockRunning = true;
}

void
onMIDIStart ()
{
	gClockRunning = true;
	gClockCount = 0;
	gShuffle = false;
	
	digitalWrite (kStartPin, HIGH);
	digitalWrite (kStartLEDPin, HIGH);
}

void
onMIDIStop ()
{
	gClockRunning = false;
	gClockCount = 0;
	gShuffle = false;
	
	digitalWrite (kStartPin, LOW);
	digitalWrite (kStartLEDPin, LOW);
        digitalWrite (kStopPin, HIGH);
        delay(5);
        digitalWrite (kStopPin, LOW);
}

