// MIDI.cpp

// INCLUDES

#include "WConstants.h" 
#include "HardwareSerial.h"
#include "MIDI.h"

// PUBLIC CONSTRUCTOR

MIDI::MIDI ()
{
}

// PUBLIC METHODS

// call from your setup() function
void
MIDI::begin (MIDIClient *inClient)
{
	this->client = inClient;

	Serial.begin (31250);
	
	this->status = 0;
	this->channel = 0;
}

// call from your loop() function
void
MIDI::poll ()
{
	while (Serial.available () > 0)
	{
		byte	midiByte = (byte) Serial.read ();
		
		// status byte?
		if (midiByte & 0x80)
		{
			// extract the channel from the channel messages
			if (midiByte < 0xf0)
			{
				// channel voice message - affects running status
				this->status = midiByte & 0xf0;
				this->channel = midiByte & 0x0f;

				dispatchDataByte (getNextDataByte ());
			}
			else
			if (midiByte >= kMIDIClock && midiByte <= kMIDIStop)
			{
				// system realtime - does not affect running status
				dispatchClockByte (midiByte);
			}
			else
			if (midiByte == kMIDISongPosition)
			{
				byte	lsb = getNextDataByte ();
				byte	msb = getNextDataByte ();
				
				if (this->client->onSongPosition)
				{
					this->client->onSongPosition (lsb, msb);
				}
			}
			else
			if (midiByte == kMIDISongSelect)
			{
				if (this->client->onSongSelect)
				{
					this->client->onSongSelect (getNextDataByte ());
				}
			}
			else
			if (midiByte == kMIDITuneRequest)
			{
				if (this->client->onTuneRequest)
				{
					this->client->onTuneRequest ();
				}
			}
			else
			if (midiByte == kMIDISysExStart)
			{
				dispatchSysExStart ();
			}
			else
			{
				// ignore everything else
			}
		}
		else
		{
			// running status
			// the byte we have is the first data byte
			dispatchDataByte (midiByte);
		}
	}
}

void
MIDI::sendChannelPressure (byte inChannel0, byte inPressure)
{
	send (kMIDIChannelPressure | (inChannel0 & 0x0f), inPressure);
}

void
MIDI::sendClock ()
{
	Serial.print (kMIDIClock, BYTE);
}

void
MIDI::sendContinue ()
{
	Serial.print (kMIDIContinue, BYTE);
}

void
MIDI::sendControlChange (byte inChannel0, byte inControl, byte inValue)
{
	send (kMIDIControlChange | (inChannel0 & 0x0f), inControl, inValue);
}

// implemented as a note on with zero velocity
// whoever came up with that needs to be hung
void
MIDI::sendNoteOff (byte inChannel0, byte inNote, byte inVelocity)
{
	send (kMIDINoteOn | (inChannel0 & 0x0f), inNote, 0);
}

void
MIDI::sendNoteOn (byte inChannel0, byte inNote, byte inVelocity)
{
	send (kMIDINoteOn | (inChannel0 & 0x0f), inNote, inVelocity);
}

void
MIDI::sendPitchBend (byte inChannel0, byte inBendLSB, byte inBendMSB)
{
	send (kMIDIPitchBend | (inChannel0 & 0x0f), inBendLSB, inBendMSB);
}

void
MIDI::sendPolyPressure (byte inChannel0, byte inNote, byte inPressure)
{
	send (kMIDIPolyPressure | (inChannel0 & 0x0f), inNote, inPressure);
}

void
MIDI::sendProgramChange (byte inChannel0, byte inProgram)
{
	send (kMIDIProgramChange | (inChannel0 & 0x0f), inProgram);
}

void
MIDI::sendSongPosition (byte inPositionLSB, byte inPositionMSB)
{
	send (kMIDISongPosition, inPositionLSB, inPositionMSB);
}

void
MIDI::sendSongSelect (byte inSongNumber)
{
	send (kMIDISongSelect, inSongNumber);
}

void
MIDI::sendStart ()
{
	Serial.print (kMIDIStart, BYTE);
}

void
MIDI::sendStop ()
{
	Serial.print (kMIDIStop, BYTE);
}

void
MIDI::sendSysExStart ()
{
	Serial.print (kMIDISysExStart, BYTE);
}

void
MIDI::sendSysExData (byte inData)
{
	Serial.print (inData, BYTE);
}

void
MIDI::sendSysExEnd ()
{
	Serial.print (kMIDISysExEnd, BYTE);
}

// PRIVATE METHODS

void
MIDI::dispatchDataByte (byte inDataByte)
{
	if (this->status == kMIDIChannelPressure)
	{
		if (this->client->onChannelPressure)
		{
			this->client->onChannelPressure (this->channel, inDataByte);
		}
	}
	else
	if (this->status == kMIDIProgramChange)
	{
		if (this->client->onProgramChange)
		{
			this->client->onProgramChange (this->channel, inDataByte);
		}
	}
	else
	if (this->status == kMIDIControlChange)
	{
		if (this->client->onControlChange)
		{
			this->client->onControlChange (this->channel, inDataByte, getNextDataByte ());
		}
	}
	else
	if (this->status == kMIDIPitchBend)
	{
		if (this->client->onPitchBend)
		{
			this->client->onPitchBend (this->channel, inDataByte, getNextDataByte ());
		}
	}
	else
	if (this->status == kMIDIPolyPressure)
	{
		if (this->client->onPolyPressure)
		{
			this->client->onPolyPressure (this->channel, inDataByte, getNextDataByte ());
		}
	}
	else
	if (this->status == kMIDINoteOff)
	{
		if (this->client->onNoteOff)
		{
			this->client->onNoteOff (this->channel, inDataByte, getNextDataByte ());
		}
	}
	else
	if (this->status == kMIDINoteOn)
	{
		byte	velocity = getNextDataByte ();
		
		// defer the zero velocity check to the client
		if (this->client->onNoteOn)
		{
			this->client->onNoteOn (this->channel, inDataByte, velocity);
		}
	}
}

void
MIDI::dispatchClockByte (byte inClockByte)
{
	if (inClockByte == kMIDIStart)
	{
		if (this->client->onStart)
		{
			this->client->onStart ();
		}
	}
	else
	if (inClockByte == kMIDIContinue)
	{
		if (this->client->onContinue)
		{
			this->client->onContinue ();
		}
	}
	else
	if (inClockByte == kMIDIStop)
	{
		if (this->client->onStop)
		{
			this->client->onStop ();
		}
	}
	else
	if (inClockByte == kMIDIClock)
	{
		if (this->client->onClock)
		{
			this->client->onClock ();
		}
	}
}

void
MIDI::dispatchSysExStart ()
{
	if (this->client->onSysExStart)
	{
		this->client->onSysExStart ();
	}
	
	byte	data = 0;
	
	do
	{
		while (Serial.available () == 0);
	
		data = Serial.read ();
		
		if (data & 0x80)
		{
			if (data < 0xf0)
			{
				// channel voice is illegal here
				// send the sysex end and update running status
				this->status = data & 0xf0;
				
				break;
			}
			else
			if (data >= kMIDIClock && data <= kMIDIStop)
			{
				// legal here
				dispatchClockByte (data);
			}
			else
			{
				// EOX terminates the loop
				// ignore everything else
			}
		}
		else
		{
			if (this->client->onSysExData)
			{
				this->client->onSysExData (data);
			}
		}
	}
	while (data != kMIDISysExEnd);

	if (this->client->onSysExEnd)
	{
		this->client->onSysExEnd ();
	}
}

byte
MIDI::getNextDataByte ()
{
	byte	data = 0;
	
	// wait for the first *data* byte
	do
	{
		while (Serial.available () == 0);
	
		data = Serial.read ();
		
		if (data & 0x80)
		{
			if (data < 0xf0)
			{
				// channel voice is illegal here
				// update running status to get back in sync
				this->status = data & 0xf0;
				this->channel = data & 0x0f;
			}
			else
			if (data >= kMIDIClock && data <= kMIDIStop)
			{
				// legal here
				dispatchClockByte (data);
			}
			else
			{
				// system common? ignore
			}
		}
	}
	while (data & 0x80);
	
	return (byte) data;
}

void
MIDI::send (byte inOne, byte inTwo)
{
	Serial.print (inOne, BYTE);
	Serial.print (inTwo, BYTE);
}

void
MIDI::send (byte inOne, byte inTwo, byte inThree)
{
	Serial.print (inOne, BYTE);
	Serial.print (inTwo, BYTE);
	Serial.print (inThree, BYTE);
}

