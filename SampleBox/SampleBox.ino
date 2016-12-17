// SampleBox.ino

// EQUATES

// standard Euro-Duino equates

const int ANALOGUE_INPUT_1 = A0;
const int ANALOGUE_INPUT_2 = A1;
const int POT_1 = A2;
const int POT_2 = A3;
const int ANALOGUE_OUTPUT_1 = 5;
const int ANALOGUE_OUTPUT_2 = 6;
const int DIGITAL_INPUT_1 = 8;
const int DIGITAL_INPUT_2 = 9;
const int DIGITAL_OUTPUT_1 = 3;
const int DIGITAL_OUTPUT_2 = 4;
const int SWITCH_1_UP = A4;
const int SWITCH_1_DOWN = A5;
const int SWITCH_2_UP = 7;
const int SWITCH_2_DOWN = 2;

// TYPES

struct SampleBox
{
  int triggerPin;
  int sampleInputPin;
  int thresholdPotPin;
  int sampleOutputPin;
  int comparatorOutputPin;

  int triggerState;
};

// PROTOTYPES

void sample (int inBoxIndex);

// GLOBAL VARIABLES

struct SampleBox gSampleBoxes [2];

// SETUP

void
setup ()
{
  // standard Euro-Duino setup
  pinMode (DIGITAL_INPUT_1, INPUT);
  pinMode (DIGITAL_INPUT_2, INPUT);
  pinMode (DIGITAL_OUTPUT_1, OUTPUT); 
  pinMode (DIGITAL_OUTPUT_2, OUTPUT); 
  pinMode (SWITCH_1_UP, INPUT_PULLUP);
  pinMode (SWITCH_1_DOWN, INPUT_PULLUP);
  pinMode (SWITCH_2_UP, INPUT_PULLUP);
  pinMode (SWITCH_2_DOWN, INPUT_PULLUP);
  
  // set up our sample boxes
  gSampleBoxes [0].triggerPin = DIGITAL_INPUT_1;
  gSampleBoxes [0].sampleInputPin = ANALOGUE_INPUT_1;
  gSampleBoxes [0].thresholdPotPin = POT_1;
  gSampleBoxes [0].sampleOutputPin = ANALOGUE_OUTPUT_1;
  gSampleBoxes [0].comparatorOutputPin = DIGITAL_OUTPUT_1;
  gSampleBoxes [0].triggerState = LOW;

  gSampleBoxes [1].triggerPin = DIGITAL_INPUT_2;
  gSampleBoxes [0].sampleInputPin = ANALOGUE_INPUT_2;
  gSampleBoxes [1].thresholdPotPin = POT_2;
  gSampleBoxes [1].sampleOutputPin = ANALOGUE_OUTPUT_2;
  gSampleBoxes [1].comparatorOutputPin = DIGITAL_OUTPUT_2;
  gSampleBoxes [1].triggerState = LOW;
  
}

// LOOP

void
loop ()
{
  sample (0);
  sample (1);
}

// FUNCTIONS

void
sample (int inBoxIndex)
{
  struct SampleBox *box = &gSampleBoxes [inBoxIndex];
   
  int trigger = digitalRead (box->triggerPin);
  
  if (trigger == HIGH && box->triggerState == LOW)
  {
    // woo, the trigger went high - it's GO time
		int	sample = random (1024); // analogRead (box->sampleInputPin);
		
		// write our sample to the analogue output
		analogWrite (box->sampleOutputPin, sample);
		
    int	threshold = analogRead (box->thresholdPotPin);
    
    // update our comparator
    // digitalWrite (box->comparatorOutputPin, sample >= threshold);
  }

  // for the moment let's have the comparator output mirror the clock
  digitalWrite (box->comparatorOutputPin, trigger);
  
  box->triggerState = trigger;
}

