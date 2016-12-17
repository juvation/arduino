// FirstMIDI.ino

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

// PROPHET PARAMETER NUMBERS

#define	VS_OSC_A_WAVE	0
#define	VS_OSC_B_WAVE	1
#define	VS_OSC_C_WAVE	2
#define	VS_OSC_D_WAVE	3

#define	VS_OSC_A_COARSE	4
#define	VS_OSC_B_COARSE	5
#define	VS_OSC_C_COARSE	6
#define	VS_OSC_D_COARSE	7

#define	VS_OSC_A_FINE 8
#define	VS_OSC_B_FINE 9
#define	VS_OSC_C_FINE 10
#define	VS_OSC_D_FINE 11

#define	VS_FILTER_CUTOFF 12
#define	VS_FILTER_RESONANCE 13
#define	VS_FILTER_ENV_AMOUNT 14

#define	VS_LFO_1_SHAPE 15
#define	VS_LFO_2_SHAPE 16
#define	VS_LFO_1_RATE 17
#define	VS_LFO_2_RATE 18

#define	VS_AMP_ENV_RATE_1	19
#define	VS_AMP_ENV_RATE_2	20
#define	VS_AMP_ENV_RATE_3	21
#define	VS_AMP_ENV_RATE_4	22
#define	VS_AMP_ENV_RATE_4A	23

#define	VS_AMP_ENV_LEVEL_0	24
#define	VS_AMP_ENV_LEVEL_1	25
#define	VS_AMP_ENV_LEVEL_2	26
#define	VS_AMP_ENV_LEVEL_3	27

// skipped loop & repeat here

#define	VS_FILTER_ENV_RATE_1	30
#define	VS_FILTER_ENV_RATE_2	31
#define	VS_FILTER_ENV_RATE_3	32
#define	VS_FILTER_ENV_RATE_4	33
#define	VS_FILTER_ENV_RATE_4A	34

#define	VS_FILTER_ENV_LEVEL_0	35
#define	VS_FILTER_ENV_LEVEL_1	36
#define	VS_FILTER_ENV_LEVEL_2	37
#define	VS_FILTER_ENV_LEVEL_3	38
#define	VS_FILTER_ENV_LEVEL_4	39

// skipped loop & repeat here

#define	VS_MIX_ENV_RATE_1	42
#define	VS_MIX_ENV_RATE_2	43
#define	VS_MIX_ENV_RATE_3	44
#define	VS_MIX_ENV_RATE_4	45
#define	VS_MIX_ENV_RATE_4A	46

#define	VS_MIX_ENV_X_LEVEL_0	47
#define	VS_MIX_ENV_X_LEVEL_1	48
#define	VS_MIX_ENV_X_LEVEL_2	49
#define	VS_MIX_ENV_X_LEVEL_3	50
#define	VS_MIX_ENV_X_LEVEL_4	51

#define	VS_MIX_ENV_Y_LEVEL_0	52
#define	VS_MIX_ENV_Y_LEVEL_1	53
#define	VS_MIX_ENV_Y_LEVEL_2	54
#define	VS_MIX_ENV_Y_LEVEL_3	55
#define	VS_MIX_ENV_Y_LEVEL_4	56

// skipped loop & repeat here

// lots skipped here

#define	VS_LFO_1_MOD_AMOUNT	96
#define	VS_LFO_2_MOD_AMOUNT	97
#define	VS_PRESSURE_MOD_AMOUNT	98
#define	VS_VELOCITY_MOD_AMOUNT	99
#define	VS_KEYBOARD_MOD_AMOUNT	100
#define	VS_FILTER_ENV_MOD_AMOUNT	101

// VIRTUAL PROPHET ENVELOPE PARAMETER NUMBERS

// note the virtual envelopes assume that level 1 == level 2 (== 99, commonly)
// upon activating AHDSR mode, level 1 and 2 are set to 99

#define VS_AMP_ENV_ATTACK	VS_AMP_ENV_RATE_1
#define VS_AMP_ENV_HOLD	VS_AMP_ENV_RATE_2
#define VS_AMP_ENV_DECAY	VS_AMP_ENV_RATE_3
#define VS_AMP_ENV_SUSTAIN	VS_AMP_ENV_LEVEL_3
#define VS_AMP_ENV_RELEASE	VS_AMP_ENV_RATE_4

#define VS_FILTER_ENV_ATTACK	VS_FILTER_ENV_RATE_1
#define VS_FILTER_ENV_HOLD	VS_FILTER_ENV_RATE_2
#define VS_FILTER_ENV_DECAY	VS_FILTER_ENV_RATE_3
#define VS_FILTER_ENV_SUSTAIN	VS_FILTER_ENV_LEVEL_3
#define VS_FILTER_ENV_RELEASE	VS_FILTER_ENV_RATE_4

// TYPEDEFS

// PROTOTYPES

void
readButtons ();

void
readButton (int inButtonPin, int inNoteNumber, int *ioNoteStatus);

void
readKnobs ();

void
readKnob (int inKnobPin, int inControlNumber, int *ioKnobValue);

void
setupParameterMap ();

// GLOBAL DATA

// caution, the buttons read opposite, so HIGH is off
int note1 = HIGH;
int note2 = HIGH;
int note3 = HIGH;

// force a change of value on the first loop
int knob1 = -100;
int knob2 = -100;

// configured by setupParameterMap()
int	parameters [48];
int	parameterValues [48];

// SETUP

void
setup ()
{
  pinMode (BUTTON_1, INPUT_PULLUP);
  pinMode (BUTTON_2, INPUT_PULLUP);
  pinMode (BUTTON_3, INPUT_PULLUP);

  pinMode (LED_GREEN, OUTPUT);
  pinMode (LED_RED, OUTPUT);
  
  // LEDs are active high for some dumb reason
  digitalWrite (LED_GREEN, HIGH);
  digitalWrite (LED_RED, HIGH);
  
  setupParameterMap ();
  
  // Launch MIDI and listen to channel 1
  MIDI.begin (1);
}

// LOOP

void
loop ()
{
  readButtons ();
  readKnobs ();
}

// 

void
readButtons ()
{
  readButton (BUTTON_1, 50, &note1);
  readButton (BUTTON_2, 60, &note2);
  readButton (BUTTON_3, 70, &note3);
}

void
readButton (int inButtonPin, int inNoteNumber, int *ioNoteStatus)
{
  int button1 = digitalRead (inButtonPin);

  // again, watch out, the buttons are active LOW sigh
  if (button1 != *ioNoteStatus)
  {
     if (button1)
     {
        MIDI.sendNoteOff (inNoteNumber, 0, 1);
        digitalWrite (LED_GREEN, 1);
     }
     else
     {
        MIDI.sendNoteOn (inNoteNumber, 100, 1);
        digitalWrite (LED_GREEN, 0);
     }

     *ioNoteStatus = button1;
  }
}

void
readKnobs ()
{
  // knob 1 = filter cutoff
  readKnob (0, 0xc, &knob1);

  // knob 2 = filter resonance
  readKnob (1, 0xd, &knob2);
}

void
readKnob (int inKnobPin, int inControlNumber, int *ioKnobValue)
{
  int value = analogRead (inKnobPin);

  // apparently the ADCs have some chatter so we shift it away
  // this has the nice side effect of making the ADCs the same resolution as the Prophet!
  value >>= 2;

  if (abs (value - *ioKnobValue) > 0)
  {
    digitalWrite (LED_RED, LOW);
    
    // parameter select 12 - filter cutoff
    MIDI.sendControlChange (0x62, inControlNumber & 0x7f, 1);
    MIDI.sendControlChange (0x63, (inControlNumber >> 8) & 0x7f, 1);
  
    // this is weird
    // MSB is bits 7:1
    int valueMSB = (value >> 1) & 0x7f;
    // LSB is bit 0 shifted
    int valueLSB = (value & 0x01) << 6;
  
    // note these are NRPNs, but little endian THANKS DAVE
    MIDI.sendControlChange (0x26, valueLSB, 1);
    MIDI.sendControlChange (0x06, valueMSB, 1);
  
    // ok now give the MIDI hardware time to de-spaz
    // delay (50);
  
    digitalWrite (LED_RED, HIGH);

    *ioKnobValue = value;
  }
}

// some parameters in play here
// questions -
// do we need all 4 coarse tunings when they will be set to the same most of the time?
//		could do A/B and C/D coarse tunings
// do we need all 4 fine tunings, or should we have a spread parameter?
//		could do A/B and C/D fine tunings
// do we need LFO shape? 
// i'm assuming that AHDSR is good enough for amp and filter envelopes
// 	otherwise each are 9 parameters!

// an alternative view
// if we're mainly concerned with waves and envelopes
// do we need the tunings at all?

void
setupParameterMap ()
{
	// do we really need all 12 oscillator parameters?
	// we effectively lose the LFO waves by doing this
	
	parameters [0] = VS_OSC_A_WAVE;
	parameters [1] = VS_OSC_B_WAVE;
	parameters [2] = VS_OSC_C_WAVE;
	parameters [3] = VS_OSC_D_WAVE;
	
	parameters [4] = VS_OSC_A_COARSE;
	parameters [5] = VS_OSC_B_COARSE;
	parameters [6] = VS_OSC_C_COARSE;
	parameters [7] = VS_OSC_D_COARSE;
	
	parameters [8] = VS_OSC_A_FINE;
	parameters [9] = VS_OSC_B_FINE;
	parameters [10] = VS_OSC_C_FINE;
	parameters [11] = VS_OSC_D_FINE;

	parameters [12] = VS_FILTER_CUTOFF;
	parameters [13] = VS_FILTER_RESONANCE;
	parameters [14] = VS_FILTER_ENV_AMOUNT;
}
