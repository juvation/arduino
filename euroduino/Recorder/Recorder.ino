// Recorder
// unrestricted use
// 05/04/15 HK initial version

// Record CV1 data, then playback at with controllable speed, note decimation and portamento
// Recording: switch #1 UP to start recording, then later switch back to Middle to stop recording
// Playback : switch #1 Down to start playback, then later switch back to Middle to stop.
//    Pot#1 controls playback speed
//    switch #2 Up : pot #2 controls length of recording
//    switch #2 Mid: pot #2 decimation of notes
//    switch #2 Dn : pot #2 controls portamento
//    DIG1out      : pulsed whenever note changes

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
  STATE_RECORDING = 0,
  STATE_PLAYBACK,
  STATE_IDLE,

  MAX_STORAGE = 295,  // low enough to avoid low memory warning
  OUTPUT_PULSE_LENGTH = 10,

  MAX_PORTAMENTO = 512,
  MAX_DECIMATION = (MAX_DAC + 1) / 4
};

typedef struct {
  byte tone;
  unsigned long tickCount;
} StorageEntry;

StorageEntry storage[MAX_STORAGE];
int sCount;                       // number entries in note storage
int scaledSCount;                 // number entries in note storage to play
int sIndex;                       // playback index into storage

byte state;
int  outPulseCount;               // #ticks remaining for digital output pulse
float pControl;                   // amount of portamento effect
float fpControl;
float pValue;                     // current portamento value.  (previous * p + input) / (p + 1)
int decimationAmount;             // amount of decimation

unsigned long tickCount;          // track duration of current playback note
int  playSpeed;                   // playback speed
byte previousTone;                // memorize last note seen during recording

// =================================================================
// initialize I/O assignments. mark note storage as empty
// =================================================================

void setup()
{
  for (int i = 0; i < CHANNELS; ++i) {
    pinMode(digitalInPort[i],  INPUT);
    pinMode(digitalOutPort[i], OUTPUT);
    pinMode(switch1Port[i],    INPUT_PULLUP);
    pinMode(switch2Port[i],    INPUT_PULLUP);
  }

  state = STATE_IDLE;
  sCount = 0;    // storage is empty

  Serial.begin(115200);
}

// =================================================================
// update switchStatus[]
// =================================================================

void readSwitches()
{
  switchStatus[CH0] = SW_MIDDLE;
  if (digitalRead(switch1Port[0]) == LOW) switchStatus[CH0] = SW_UP;
  if (digitalRead(switch1Port[1]) == LOW) switchStatus[CH0] = SW_DN;

  switchStatus[CH1] = SW_MIDDLE;
  if (digitalRead(switch2Port[0]) == LOW) switchStatus[CH1] = SW_UP;
  if (digitalRead(switch2Port[1]) == LOW) switchStatus[CH1] = SW_DN;
}

// =================================================================
// map full range of cvIn1 to analog OUT
// =================================================================

byte currentReading()
{
  float v = (float)analogRead(cvInPort[CH0]) * (float)MAX_DAC / (float)MAX_ADC;
  return (byte)v;
}

// =================================================================
// reset note storage count and note duration counter
// update state
// =================================================================

void startRecording()
{
  sCount  = 0;
  sIndex = 0;    // use channel #0 counters to track recording progress
  tickCount = 0;

  state = STATE_RECORDING;

  previousTone = currentReading(); // recording will not start until first note change is detected
}

// =================================================================
// store duration of last note; update processing flag
// =================================================================

void stopRecording()
{
  if (sCount > 0) // duration of last note before Recording session is switched off
    storage[sCount - 1].tickCount = tickCount;

  state = STATE_IDLE; 
}

// =================================================================
// increment duration counter for current note
// current note is CV1in
// when note changes: store duration of note that just ended,
//    add new entry to storage for note just detected
//    play new note with trigger
// =================================================================

void recordingProcess()
{
  if (sCount == MAX_STORAGE) return; // no more room

  ++tickCount; // update duration of current note

  byte tone = currentReading();

  // adjust this test for decimationAmountization effects
  bool hasNoteChanged = abs(tone - previousTone) > 2;

  if (hasNoteChanged) {
    if (sCount > 0) // we finally know the duration of the previous note
      storage[sCount - 1].tickCount = tickCount;

    storage[sCount++].tone = tone; // add new entry to storage (no duration yet)

    tickCount = 0;  // reset duration counter for this new note
    previousTone = tone; // so storage is only updated when the note changes

    aOutput(cvOutPort[CH0], tone);
    startTriggerOut();
  }
}

// =================================================================
// initiate trigger on DigOut #1
// =================================================================

void startTriggerOut()
{
  outPulseCount = OUTPUT_PULSE_LENGTH;
  digitalWrite(digitalOutPort[CH0], HIGH);
}

// =================================================================
// play current playback note and decimation and portamento
// begin trigger pulse if directed
// =================================================================

void playCurrentStorageEntry(bool addTrigger)
{
  byte tone = storage[sIndex].tone;
    
    int v = tone - 3 + random(0,7);
    if(v < 0) v=0; else if(v > 255) v = 255;
    tone = (byte)v;
    storage[sIndex].tone = tone;
    
    int w = storage[sIndex].tickCount - 3 + random(0,7);
    if(w < 1) w = 1;
    storage[sIndex].tickCount = w;
   
  // apply decimationAmountatization
  if (decimationAmount > 0) {
    int v = (((int)tone + decimationAmount / 2) / decimationAmount) * decimationAmount;
    if (v > MAX_DAC)
      v = MAX_DAC;
    tone = (byte)v;
  }

  // apply portamento
  if (pControl > 0) {
    pValue = (pValue * fpControl + (float)tone) / (fpControl + 1.0f);
    tone = (byte)pValue;
  }

  aOutput(cvOutPort[CH0], tone);

  if (addTrigger)
    startTriggerOut();
}

// =================================================================
// reset index into note storage
// reset playback controls. play first note.
// update playback processing flag
// =================================================================

void startPlayback()
{
  if (state == STATE_RECORDING || !sCount) return;

  sIndex = 0;             // start at beginning of recording
  scaledSCount = sCount;  // default to playing all of recording
  tickCount = 0;          // reset duration of first note
  pValue = 0;             // default to portamento disabled
  pControl = 0;
  decimationAmount = 0;   // default to decimation disabled

  playCurrentStorageEntry(true);
  state = STATE_PLAYBACK;
}

// =================================================================
// update playback processing flag
// stop playback of current note
// =================================================================

void stopPlayback()
{
  state = STATE_IDLE;
  aOutput(cvOutPort[CH0], 0);
}

// =================================================================
// speed up / slow down playback rate by adjusting note duration
// according to specified rate factor
// =================================================================

unsigned long scaledStorageDuration (
  int rateFactor,          // 0..MAX_ADC reading of rate Pot
  unsigned long duration)  // true duration of current note
{
  const float center = (float)MAX_ADC / 2.0f;
  float scale, ratio = ((float)rateFactor - center) / center;

  if (ratio >= 0.0f)  // smaller 'scale' = faster playback
    scale = 1.0f - 0.95f * ratio;
  else                // larger 'scale' = slower playback
    scale = 1.0f - 9.0f * ratio;

  return (unsigned long)((float)duration * scale);
}

// =================================================================
// if scaled duration of current note has expired then
// advance to next note in storage (round-robin)
// while dwelling on current note, apply portamento
// =================================================================

int pot[2],oldPot[2];

void playbackProcess()
{
  pot[0] = analogRead(potPort[CH0]);
  pot[1] = analogRead(potPort[CH1]);
  
  if (abs(pot[0] - oldPot[0]) > 2) {
    oldPot[0] = pot[0];
    playSpeed = pot[0];
  }
  
  // switch2 controls during playback
  if (abs(pot[1] - oldPot[1]) > 2) {
    oldPot[1] = pot[1];

    switch (switchStatus[CH1]) {
      case SW_UP :
        scaledSCount = (int)((float)sCount * (float)pot[1] / (float)MAX_ADC);
        if (sCount > 2 && scaledSCount < 2) scaledSCount = 2;
        else if (scaledSCount > sCount) scaledSCount = sCount;
        break;
      case SW_MIDDLE :
        decimationAmount = (int)((((float)pot[1] * (float)MAX_DECIMATION)) / (float)MAX_ADC);
        break;
      default :
        pControl = pot[1]/2;  // 0..MAX_PORTAMENTO
        fpControl = (float)pControl;
        break;
    }
  }
  
  unsigned long scaledDuration = scaledStorageDuration(playSpeed, storage[sIndex].tickCount);

  if (++tickCount < scaledDuration) { // duration has not expired yet
    // apply portamento
    if (pControl > 0)
      playCurrentStorageEntry(false);

    return;
  }

  if (++sIndex >= scaledSCount) // point at next note in storage (round-robin)
    sIndex = 0;

  tickCount = 0;  // reset duration counter for this new note

  playCurrentStorageEntry(true);
}

// =================================================================
// during idle pass detected note changes to CVout along with a trigger
// =================================================================

void idleProcess()
{
  byte tone = currentReading();

  if (abs(tone - previousTone) > 2) {
    previousTone = tone;
    aOutput(cvOutPort[CH0], tone);
    startTriggerOut();
  }
}

// =================================================================

void loop()
{
  readSwitches();

  // sw1 UP = start recording ------------------------------------
  if (state != STATE_RECORDING && switchStatus[CH0] == SW_UP)
    startRecording();
  else if (state == STATE_RECORDING && switchStatus[CH0] != SW_UP)
    stopRecording();

  // sw1 Dn = start playback. ------------------------------------
  if (state != STATE_PLAYBACK && switchStatus[CH0] == SW_DN)
    startPlayback();
  else if (state == STATE_PLAYBACK && switchStatus[CH0] != SW_DN)
    stopPlayback();

  switch (state) {
    case STATE_RECORDING :
      recordingProcess();
      break;
    case STATE_PLAYBACK :
      playbackProcess();
      break;
    case STATE_IDLE :
      idleProcess();
      break;
  }

  // has output pulse time expired?
  if (outPulseCount > 0) {
    --outPulseCount;
    if (outPulseCount == 0)
      digitalWrite(digitalOutPort[CH0], LOW);
  }

  delay(2);
}




