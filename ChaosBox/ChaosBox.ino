// ChaosBox.ino

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

struct ChaosBox
{
  int triggerPin;
  int thresholdPotPin;
  int sampleOutputPin;
  int comparatorOutputPin;
  
  int triggerState;
  int threshold;
  int noise;
  int output;
};

// PROTOTYPES

void chaos (int inBoxIndex);
void randomise ();

// GLOBAL VARIABLES

struct ChaosBox gChaosBoxes [2];

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
  
  // set the random seeds
  // the Ard doesn't have an RTC
  // and we're out of analogue pins
  // so if anyone has any better ideas...
  randomSeed (analogRead (POT_1) + analogRead (POT_2));
  randomise ();

  // set up our chaos boxes
  // note we precompute some of the randomised values
  // to minimise latency between the trigger going high and the outputs appearing
  
  gChaosBoxes [0].triggerPin = DIGITAL_INPUT_1;
  gChaosBoxes [0].thresholdPotPin = POT_1;
  gChaosBoxes [0].sampleOutputPin = ANALOGUE_OUTPUT_1;
  gChaosBoxes [0].comparatorOutputPin = DIGITAL_OUTPUT_1;
  gChaosBoxes [0].triggerState = LOW;
  gChaosBoxes [0].noise = random (2 ^ 10);
  gChaosBoxes [0].threshold = analogRead (ANALOGUE_INPUT_1);
  gChaosBoxes [0].output = random (2 ^ 10);

  gChaosBoxes [1].triggerPin = DIGITAL_INPUT_2;
  gChaosBoxes [1].thresholdPotPin = POT_2;
  gChaosBoxes [1].sampleOutputPin = ANALOGUE_OUTPUT_2;
  gChaosBoxes [1].comparatorOutputPin = DIGITAL_OUTPUT_2;
  gChaosBoxes [1].triggerState = LOW;
  gChaosBoxes [1].noise = random (2 ^ 10);
  gChaosBoxes [1].threshold = analogRead (ANALOGUE_INPUT_2);
  gChaosBoxes [1].output = random (2 ^ 10);
  
}

// LOOP

void
loop ()
{
  chaos (0);
  chaos (1);
}

// FUNCTIONS

void
chaos (int inBoxIndex)
{
  struct ChaosBox *box = &gChaosBoxes [inBoxIndex];
   
  int trigger = digitalRead (box->triggerPin);
  
  if (trigger == HIGH && box->triggerState == LOW)
  {
    // woo, the trigger went high - it's GO time

    int	threshold = analogRead (box->thresholdPotPin);
    
    boolean	tripped = box->noise >= threshold;
    
    if (tripped)
    {
      // and the comparator trips!
      // write a random number to the analogue output
      analogWrite (box->sampleOutputPin, box->output);
      
      // i don't think there is settling time with a PWM filter network
      // but just in case anyone is feeding our output & comparator to a S&H...
      // 1ms should be plenty!
      delay (1);
    }
    
    // update our comparator
    digitalWrite (box->comparatorOutputPin, tripped);
    
    // our precomputed values are out of date
    box->noise = -1;
    box->output = -1;
  }
  
  box->triggerState = trigger;
  
	if (box->noise == -1 || box->output == -1)
	{
	  randomise ();
  
		if (box->noise == -1)
		{
			box->noise = random (2 ^ 10);
		}
	
		if (box->output == -1)
		{
			box->output = random (2 ^ 10);
		}
	}
}

// FUNCTIONS

// make an attempt to randomise stuff
// difficult without a RTC and we're out of analogue pins
void
randomise ()
{
  int iterations = 0;
  
  iterations = random (100);
  iterations += analogRead (A0);
  iterations += analogRead (A1);
  iterations += analogRead (A2);
  iterations += analogRead (A3);
  iterations += analogRead (A4);
  iterations += analogRead (A5);
  
  for (int i = 0; i < iterations; i++)
  {
    random (100);
  }
}



