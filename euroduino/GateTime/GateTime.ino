// trigger to gate converter
// with VC and manual gate time
// past a certain point it's a tie
// but since we don't know what the gap to the next note is...
// past zero (or maybe near zero) it's a rest

// These constants won't change.  They're used to give names
// to the pins used:
const int kAnalogIn1Pin = A0;  // Analog Input 1
const int kAnalogIn2Pin = A1; // Analog Input 2
const int kAnalogPot1Pin = A2;  // Pot 1
const int kAnalogPot2Pin = A3; // Pot 2
const int kAnalogOut1Pin = 5;  // Analog Output 1
const int kAnalogOut2Pin = 6; // Analog Output 2
const int kDigitalIn1Pin = 8;  // Digital Input 1
const int kDigitalIn2Pin = 9;  // Digital Input 2
const int kDigitalOut1Pin = 3;  // Digital Output 1
const int kDigitalOut2Pin = 4;  // Digital Output 2
const int kSwitch1UpPin = A4;  // Switch 1 Up
const int kSwitch1DownPin = A5;  // Switch 1 Dwn
const int kSwitch2UpPin = 7;  // Switch 2 Up
const int kSwitch2DownPin = 2;  // Switch 2 Dwn

// TYPEDEFS

typedef struct
{
	// config

	int 					mGateInPin;
	int 					mGateTimeCVPin;
	int 					mGateTimePotPin;
	int 					mGateOutPin;

	// state

	int 					mGateIn;
	unsigned long	mGateInTime;
	int 					mGateOut;
	unsigned long	mGateOutTime;

	unsigned long	mGateTime;
}
Channel;

// GLOBALS

Channel gChannels[2];

// FUNCTIONS

int convertToTicks(int inGateTime)
{
	// for now hardwire this :-)
	return inGateTime;
}

int getGateTime(int inChannel)
{
	Channel	*channel = &gChannels[inChannel];

	int	potGateTime = analogRead(channel->mGateTimePotPin);
	int	cvGateTime = analogRead(channel->mGateTimeCVPin);
	int	gateTime = potGateTime + cvGateTime;

	return convertToTicks(gateTime);
}

void loopChannel(int inNow, int inChannel)
{
	Channel	*channel = &gChannels[inChannel];

	// resample the gate time
	// note this needs to be converted to now() ticks
	channel->mGateTime = getGateTime(inChannel);

	int gateOn = digitalRead(channel->mGateInPin);

	// if we are sending a gate
	if (channel->mGateOut)
	{
		// and a new gate is received
		if (gateOn && !channel->mGateIn)
		{
			// extend the gate time
			channel->mGateOutTime = inNow;
		}
		else if (inNow >= channel->mGateOutTime + channel->mGateTime)
		{
			// it's time
			digitalWrite(channel->mGateOutPin, 0);

			channel->mGateOut = false;
			channel->mGateOutTime = 0;
		}
	}
	else
	{
		if (gateOn)
		{
			// respect the glorious rest
			if (channel->mGateTime > 100)
			{
				digitalWrite(channel->mGateOutPin, 1);

				channel->mGateOut = true;
				channel->mGateOutTime = inNow;
			}

		}			
	}

	channel->mGateIn = gateOn;
}

// SETUP

void setup()
{
	Serial.begin(9600);

	pinMode(kDigitalIn1Pin, INPUT);
	pinMode(kDigitalIn2Pin, INPUT);
	pinMode(kDigitalOut1Pin, OUTPUT); 
	pinMode(kDigitalOut2Pin, OUTPUT); 
	pinMode(kSwitch1UpPin, INPUT_PULLUP);
	pinMode(kSwitch1DownPin, INPUT_PULLUP);
	pinMode(kSwitch2UpPin, INPUT_PULLUP);
	pinMode(kSwitch2DownPin, INPUT_PULLUP);

	gChannels[0].mGateInPin = kDigitalIn1Pin;
	gChannels[0].mGateTimeCVPin = kAnalogIn1Pin;
	gChannels[0].mGateTimePotPin = kAnalogPot1Pin;
	gChannels[0].mGateOutPin = kDigitalOut1Pin;
	gChannels[0].mGateIn = false;
	gChannels[0].mGateInTime = 0;
	gChannels[0].mGateOut = false;
	gChannels[0].mGateOutTime = 0;
	gChannels[0].mGateTime = 0;

	gChannels[1].mGateInPin = kDigitalIn2Pin;
	gChannels[1].mGateTimeCVPin = kAnalogIn2Pin;
	gChannels[1].mGateTimePotPin = kAnalogPot2Pin;
	gChannels[1].mGateOutPin = kDigitalOut2Pin;
	gChannels[1].mGateIn = false;
	gChannels[1].mGateInTime = 0;
	gChannels[1].mGateOut = false;
	gChannels[1].mGateOutTime = 0;
	gChannels[1].mGateTime = 0;

	for (int i = 0; i < 2; i++)
	{
		digitalWrite(gChannels[i].mGateOutPin, 0);
	}

	// they tell me the analogue outputs are inverted, wtf
	analogWrite(kAnalogOut1Pin, 1023);
	analogWrite(kAnalogOut2Pin, 1023);
}

void loop()
{
	unsigned long now = millis();

	for (int i = 0; i < 2; i++)
	{
			loopChannel(now, i);
	}
}

