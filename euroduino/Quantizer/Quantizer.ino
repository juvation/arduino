// Quantizer
// unrestricted use
// 05/05/15 HK initial version

// Pots control quantization, limiting and inverting of CVins as they are copied to CVouts
// SW1 UP      : pot1 controls High level of CVin1
// SW1 DN      : pot1 controls Low level of CVin1
// SW1 MIDDLE  : pot1 controls Quantization of CVin1
// Dig1 is HIGH: pot1 controls portamento for channel1

// SW2,pot2,Dig2 do the same for CVin2 -> CVout2

// COMMON ========================================================
enum {
  SW_UP = 0,
  SW_DN,
  SW_MIDDLE,
  CHANNELS = 2,
  CH0 = 0,
  CH1,
  MAX_ADC = 1023,   // max analog input value
  MAX_DAC = 255,    // max analog output value
};

// analog output ports are inverted
#define aOutput(port,value) analogWrite(port,MAX_DAC - (value))

byte switchStatus[CHANNELS];
const byte switch1Port[CHANNELS]    = { A4, A5 };
const byte switch2Port[CHANNELS]    = { 7, 2 };
const byte cvInPort[CHANNELS]       = { A0, A1 };
const byte cvOutPort[CHANNELS]      = { 5, 6 };
const byte potPort[CHANNELS]        = { A2, A3 };
const byte digitalInPort[CHANNELS]  = { 8, 9 };
const byte digitalOutPort[CHANNELS] = { 3, 4 };
// COMMON ========================================================

enum {
  BIT_MASK_SIZE = 8
};

byte  digitalIn[CHANNELS];        // current digitalInCV IN status
int   lowLimit[CHANNELS];         // output will be scaled to >= this level
int   highLimit[CHANNELS];        // output will be scaled to <= this level
float pControl[CHANNELS];         // amount of portamento effect (pot reading)
float pValue[CHANNELS];           // current portamento value.  value = (previous * p + input) / (p + 1)
float scaleFactor[CHANNELS];      // amount to scale input to fit within limits
byte  quantizationMask[CHANNELS]; // current bit mask

byte qMask[BIT_MASK_SIZE] = { 0b11111111, 0b11111110, 0b11111100, 0b11111000, 0b11110000, 0b11100000, 0b11000000, 0b10000000 };

// =================================================================
// initialize I/O assignments, set initial scale and portamento factors
// =================================================================

void setup()
{
  for (int i = 0; i < CHANNELS; ++i) {
    pinMode(digitalInPort[i],  INPUT);
    pinMode(digitalOutPort[i], OUTPUT);
    pinMode(switch1Port[i],    INPUT_PULLUP);
    pinMode(switch2Port[i],    INPUT_PULLUP);
  }

  for (byte channel = 0; channel < CHANNELS; ++channel) {
    scaleFactor[channel] = 1.0f;
    pControl[channel] = 0;
    pValue[channel] = 0;
  }
}

// =================================================================
// update switchStatus[] and digitalIn[] input status
// =================================================================

void readSwitches()
{
  switchStatus[CH0] = SW_MIDDLE;
  if (digitalRead(switch1Port[0]) == LOW) switchStatus[CH0] = SW_UP;
  if (digitalRead(switch1Port[1]) == LOW) switchStatus[CH0] = SW_DN;

  switchStatus[CH1] = SW_MIDDLE;
  if (digitalRead(switch2Port[0]) == LOW) switchStatus[CH1] = SW_UP;
  if (digitalRead(switch2Port[1]) == LOW) switchStatus[CH1] = SW_DN;

  for (int i = 0; i < CHANNELS; ++i)
    digitalIn[i] = digitalRead(digitalInPort[i]);
}

// =================================================================

void monitorPot(
  byte channel)
{
  int pot = analogRead(potPort[channel]);

  if (digitalIn[channel] == HIGH)
    pControl[channel] = (float)pot;
  else {
    switch (switchStatus[channel]) {
      case SW_UP :
        highLimit[channel] = pot;
        break;

      case SW_DN :
        lowLimit[channel] = pot;
        break;

      default :
        quantizationMask[channel] = qMask[pot * BIT_MASK_SIZE / MAX_ADC];
        break;
    }

    if (switchStatus[channel] != SW_MIDDLE)
      scaleFactor[channel] = (float)(highLimit[channel] - lowLimit[channel]) / (float)MAX_ADC; // -1 ... +1
  }
}

// =================================================================

void updateOutput(
  byte channel)
{
  int input = analogRead(cvInPort[channel]);

  // apply portamento
  if (pControl[channel] > 0) {
    pValue[channel] = (pValue[channel] * pControl[channel] + (float)input) / (pControl[channel] + 1.0f);
    input = (int)pValue[channel];
  }

  byte quantitizedInput = (byte)(input & quantizationMask[channel]);
  float scaledInput = (float)quantitizedInput * scaleFactor[channel];

  aOutput(cvOutPort[channel], lowLimit[channel] + (byte)scaledInput);
}

// =================================================================

int pace,hk;

void loop()
{
  readSwitches();

  for (byte i = 0; i < CHANNELS; ++i) {
    monitorPot(i);
    updateOutput(i);
  }
 
  delay(2);
}



