// Sequencer Version 1

// COMMON ========================================================
enum {
  SW_UP = 0,
  SW_DN,
  SW_MIDDLE,
  CHANNELS = 2,
  CH0 = 0,
  CH1,
  MAX_ADC = 1023, // max analog input value
  MAX_DAC = 255,  // max analog output value
};

// analog output ports are inverted
#define aOutput(port,value) analogWrite(port,MAX_DAC - (value))

byte switchStatus[CHANNELS];
byte digStatus[CHANNELS];
byte oldSw[CHANNELS], oldDig[CHANNELS];
int oldPot[CHANNELS], oldCvIn[CHANNELS];
const byte switch1Port[CHANNELS] = { A4, A5 };
const byte switch2Port[CHANNELS] = { 7, 2 };
const byte cvInPort[CHANNELS] = { A0, A1 };
const byte cvOutPort[CHANNELS] = { 5, 6 };
const byte potPort[CHANNELS] = { A2, A3 };
const byte digitalInPort[CHANNELS] = { 8, 9 };
const byte digitalOutPort[CHANNELS] = { 3, 4 };
// COMMON ========================================================

enum {
  STX = 2,
  ETX = 3,
  SEQUENCER_INPUT = 0x57,
  SEQUENCER_OUTPUT = 0x58
};

typedef struct {
  byte  stx;
  byte  typecode;
  byte  cvOut1;
  byte  cvOut2;
  byte  digOut1;
  byte  digOut2;
  byte  etx;
} SequenceInput;

typedef struct {
  byte  stx;
  byte  typecode;
  int   pot1;
  int   pot2;
  int   cvIn1;
  int   cvIn2;
  byte  sw1;
  byte  sw2;
  byte  digIn1;
  byte  digIn2;
  byte  etx;
} SequenceOutput;

#define PACKET_INPUT  sizeof(SequenceInput)
#define PACKET_OUTPUT sizeof(SequenceOutput)

SequenceInput  p;
SequenceOutput po;

// =================================================================
// initialize I/O assignments
// =================================================================

void setup()
{
  for (int i = 0; i < CHANNELS; ++i) {
    pinMode(digitalInPort[i], INPUT);
    pinMode(digitalOutPort[i], OUTPUT);
    pinMode(switch1Port[i], INPUT_PULLUP);
    pinMode(switch2Port[i], INPUT_PULLUP);
  }

  Serial.begin(115200);
}

// =================================================================
// update switch status (SW_UP,SW_DN,SW_MIDDLE)
// =================================================================

void readSwitches()
{
  switchStatus[CH0] = SW_MIDDLE;
  if (digitalRead(switch1Port[0]) == LOW) switchStatus[CH0] = SW_UP;
  if (digitalRead(switch1Port[1]) == LOW) switchStatus[CH0] = SW_DN;
  switchStatus[CH1] = SW_MIDDLE;
  if (digitalRead(switch2Port[0]) == LOW) switchStatus[CH1] = SW_UP;
  if (digitalRead(switch2Port[1]) == LOW) switchStatus[CH1] = SW_DN;

  digStatus[CH0] = digitalRead(digitalInPort[CH0]);
  digStatus[CH1] = digitalRead(digitalInPort[CH1]);
}

// =================================================================
// packet bytes received.  check for valid packet,
// copy contents to local storage, update calculated variables
// =================================================================

byte receiveBuffer[PACKET_INPUT + 1];

void parsePacket()
{
  if (receiveBuffer[0] != STX) return;       // bad packet?
  if (receiveBuffer[1] != SEQUENCER_INPUT) return;
  if (receiveBuffer[PACKET_INPUT - 1] != ETX) return;

  memcpy(&p, receiveBuffer, PACKET_INPUT);

  // update ports --------
  aOutput(cvOutPort[0], p.cvOut1);
  aOutput(cvOutPort[1], p.cvOut2);
  digitalWrite(digitalOutPort[CH0], p.digOut1);
  digitalWrite(digitalOutPort[CH1], p.digOut2);
}

// =================================================================
// monitor serial received data. when STX is detected, collect succeeding
// bytes to buffer, parse when filled.
// =================================================================

byte receiveState = 0;
byte bufferIndex = 0;

void monitorSerialReception()
{
  byte ch;

  for (;;) {
    if (Serial.available() <= 0) return;
    ch = Serial.read();

    if (receiveState == 0 && ch == STX) {
      receiveState = 1;
      bufferIndex = 0;
    }

    if (receiveState == 1) {
      receiveBuffer[bufferIndex] = ch;
      if (++bufferIndex >= PACKET_INPUT) {
        parsePacket();
        receiveState = 0;
      }
    }
  }
}

// =================================================================

void sendOutputPacket()
{
  po.stx = STX;
  po.typecode = SEQUENCER_OUTPUT;
  po.pot1 = analogRead(potPort[CH0]);
  po.pot2 = analogRead(potPort[CH1]);
  po.cvIn1 = analogRead(cvInPort[CH0]);
  po.cvIn2 = analogRead(cvInPort[CH1]);
  po.sw1 = switchStatus[CH0];
  po.sw2 = switchStatus[CH1];
  po.digIn1 = digStatus[CH0];
  po.digIn2 = digStatus[CH1];
  po.etx = ETX;

  Serial.write((byte *)&po, PACKET_OUTPUT);
}

// =================================================================

void loop()
{
  int pot[CHANNELS], cv[CHANNELS];

  readSwitches();
  pot[0] = analogRead(potPort[CH0]);
  pot[1] = analogRead(potPort[CH1]);
  cv[0] =  analogRead(cvInPort[CH0]);
  cv[1] =  analogRead(cvInPort[CH1]);

  // only send output packet when changed ----------------------
  bool changed = false;
  for (int i = 0; i < CHANNELS; ++i) {
    if (oldSw[i] != switchStatus[i]) {
      oldSw[i] = switchStatus[i];
      changed = true;
    }
    if (oldDig[i] != digStatus[i]) {
      oldDig[i] = digStatus[i];
      changed = true;
    }
    if (abs(oldPot[i] - pot[i]) > 3) { // allow for jitter in ADC reading
      oldPot[i] = pot[i];
      changed = true;
    }
    if (abs(oldCvIn[i] - cv[i]) > 3) { // allow for jitter in ADC reading
      oldCvIn[i] = cv[i];
      changed = true;
    }
  }

  if (changed)
    sendOutputPacket();

  delay(2);
  monitorSerialReception();
}

