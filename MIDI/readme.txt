How to use the MIDI Library
----------------------------

1. Unpack MIDI.zip and put the MIDI directory in your hardware/libraries directory.

2. Include "MIDI.h" in your sketch.

3. Declare a global variable of type MIDI called MIDI in your sketch.
I'd do this for you in the library, but the compiler won't let me.

4. Declare a global variable of type MIDIClient in your sketch. Check MIDI.h
for what to put in there - basically pointers to your functions that handle
various types of MIDI message.

3. Call MIDI.begin(&midiClient) in your setup() function.

4. Call MIDI.poll() in your loop() function.



