// MIDI.h

#ifndef	MIDI_h
#define MIDI_h

// DEFINES

#define kMIDINoteOff 0x80
#define kMIDINoteOn 0x90
#define kMIDIPolyPressure 0xa0
#define kMIDIControlChange 0xb0
#define kMIDIProgramChange 0xc0
#define kMIDIChannelPressure 0xd0
#define kMIDIPitchBend 0xe0
#define kMIDISysExStart 0xf0
#define kMIDISongPosition 0xf2
#define kMIDISongSelect 0xf3
#define kMIDITuneRequest 0xf6
#define kMIDISysExEnd 0xf7
#define kMIDIClock 0xf8
#define kMIDIStart 0xfa
#define kMIDIContinue 0xfb
#define kMIDIStop 0xfc
#define kMIDIActiveSensing 0xfe
#define kMIDIReset 0xff

// TYPEDEFS

typedef struct
{
	void
	(*onActiveSensing) ();

	void
	(*onChannelPressure) (byte inChannel0, byte inPressure);

	void
	(*onClock) ();

	void
	(*onContinue) ();

	void
	(*onControlChange) (byte inChannel0, byte inControl, byte inValue);

	void
	(*onNoteOff) (byte inChannel0, byte inNote, byte inVelocity);

	void
	(*onNoteOn) (byte inChannel0, byte inNote, byte inVelocity);

	void
	(*onPitchBend) (byte inChannel0, byte inBendLSB, byte inBendMSB);

	void
	(*onPolyPressure) (byte inChannel0, byte inNote, byte inPressure);

	void
	(*onProgramChange) (byte inChannel0, byte inProgram);

	void
	(*onReset) ();

	void
	(*onSongPosition) (byte inPositionLSB, byte inPositionMSB);

	void
	(*onSongSelect) (byte inSongNumber);

	void
	(*onStart) ();

	void
	(*onStop) ();

	void
	(*onSysExStart) ();

	void
	(*onSysExData) (byte inData);

	void
	(*onSysExEnd) ();

	void
	(*onTuneRequest) ();

}
MIDIClient;

// IMPORTS

// implement these to use the MIDI library

// CLASS

class MIDI
{
	// public constructors
	public:
		
		MIDI ();
		
	// public methods
	public:
	
		void
		begin (MIDIClient *inClient);
		
		void
		poll ();
		
		void
		sendChannelPressure (byte inChannel0, byte inPressure);

		void
		sendClock ();
		
		void
		sendContinue ();
		
		void
		sendControlChange (byte inChannel0, byte inControl, byte inValue);
		
		void
		sendNoteOff (byte inChannel0, byte inNote, byte inVelocity);
		
		void
		sendNoteOn (byte inChannel0, byte inNote, byte inVelocity);
		
		void
		sendPitchBend (byte inChannel0, byte inBendLSB, byte inBendMSB);
		
		void
		sendPolyPressure (byte inChannel0, byte inNote, byte inPressure);
		
		void
		sendProgramChange (byte inChannel0, byte inProgram);
		
		void
		sendSongPosition (byte inPositionLSB, byte inPositionMSB);
		
		void
		sendSongSelect (byte inSongNumber);
	
		void
		sendStart ();
		
		void
		sendStop ();
		
		void
		sendSysExStart ();
	
		void
		sendSysExData (byte inData);
	
		void
		sendSysExEnd ();

	// private methods
	private:
	
		void
		dispatchDataByte (byte inDataByte);
		
		void
		dispatchClockByte (byte inClockByte);
		
		void
		dispatchSysExStart ();
		
		byte
		getNextDataByte ();
		
		void
		send (byte inOne, byte inTwo);
		
		void
		send (byte inOne, byte inTwo, byte inThree);
		
	// private data
	private:
	
		MIDIClient *
		client;
		
		byte
		channel;
		
		byte
		status;
		
};

#endif	// MIDI_h

