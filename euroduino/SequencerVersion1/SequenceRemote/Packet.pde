final byte STX = 2;
final byte ETX = 3;
final byte SEQUENCER_INPUT = 0x57;   // packet type code
final byte SEQUENCER_OUTPUT = 0x58;

// ==================================================================
// ask user which COM port to use
// ==================================================================
String COMx, COMlist = "";

Boolean askForPort = true; // false = automatically select last listed port

void selectSerialPort()
{
  try {
    printArray(Serial.list());
    int i = Serial.list().length;
    if (i != 0) {
      if (askForPort) {
        if (i >= 2) {
          for (int j = 0; j < i; ) {
            COMlist += char(j+'a') + " = " + Serial.list()[j];
            if (++j < i) COMlist += ", \n";
          }
          COMx = showInputDialog("Which COM port connects to Orgone Accumulator? (a,b,..):\n"+COMlist);
          if (COMx == null) exit();
          if (COMx.isEmpty()) exit();
          i = int(COMx.toLowerCase().charAt(0) - 'a') + 1;
        }
      }
      String portName = Serial.list()[i-1];
      println(portName);
      port = new Serial(this, portName, 115200);
      port.buffer(10000);
    } else {
      showMessageDialog(frame, "Device is not connected to the PC");
      exit();
    }
  }
  catch (Exception e)
  { 
    //showMessageDialog(frame, "COM port is not available (maybe in use by another program)");
    //println("Error:", e);
    exit();
  }
}

// ==================================================================
// send Sequence packet to EuroDuino
// ==================================================================

byte[] packet = new byte[NUM_CV+3];  // +3 = stx,typecode,etx
boolean packetSent = false;

void sendSequencePacket(SequencePacket pp)
{
  if (port == null) return;

  int index = 0;
  packet[index++] = STX;
  packet[index++] = SEQUENCER_INPUT;    // packet is 'input' from EuroDuino point of view

  for (int i=0; i<NUM_CV; ++i) 
    packet[index++] = pp.data[i];

  packet[index] = ETX;
  port.write(packet);

  packetSent = pp.data[2] > 0 || pp.data[3] > 0; // so a succeeding 'low gate' packet will be sent

  // println("sent: " + byteToInt(pp.data[0]) + ", " + byteToInt(pp.data[1]) + ", " + pp.data[2] + ", " + pp.data[3]);
}

// ==================================================================

void sendLowGate()
{
  if (port == null) return;
  if (!packetSent) return;  // send LOW gate to terminate previous Gate High
  packetSent = false;

  packet[4] = 0; // digout1 position in packet (stx,type,cv1,cv2,*
  packet[5] = 0; // digout2

  port.write(packet);
}

// ==================================================================
// convert two bytes read from serial port into 'int' value
// ==================================================================

byte lowByte, highByte;

int bytesToInt()
{
  int v = byteToInt(lowByte);

  return v + (int)highByte * 256;
}

// ==================================================================
// write any chars received from EutoDuino to console
// ==================================================================

int state = 0;
int count;

void monitorSerialPort() 
{
  if (port == null) return;

  while (port.available () > 0) {
    int val = port.read();
    byte bval = (byte)val;

    switch(state) {
    case 0 :
      if (bval == STX) {
        cp.stx = bval;
        state = 1;
      } else {
        print((char)bval); // not packet data, display on console
      }
      break;
    case 1 :
      if (bval == SEQUENCER_OUTPUT) {  // output packet from E's point of view
        cp.typecode = bval;
        state = 2;
        count = 0;
      } else
        state = 0; // unrecognized typecode  
      break;

      // pot1 ------------------
    case 2 :
      lowByte = bval;
      ++state;
      break;
    case 3 :
      highByte = bval;
      cp.pot1 = bytesToInt();
      ++state;
      break;

      // pot2 ------------------
    case 4 :
      lowByte = bval;
      ++state;
      break;
    case 5 :
      highByte = bval;
      cp.pot2 = bytesToInt();
      ++state;
      break;

      // cvIn1 ------------------
    case 6 :
      lowByte = bval;
      ++state;
      break;
    case 7 :
      highByte = bval;
      cp.cvIn1 = bytesToInt();
      ++state;
      break;

      // cvIn2 ------------------
    case 8 :
      lowByte = bval;
      ++state;
      break;
    case 9 :
      highByte = bval;
      cp.cvIn2 = bytesToInt();
      ++state;
      break;

    case 10 :
      cp.sw1 = bval;
      ++state;
      break;
    case 11 :
      cp.sw2 = bval;
      ++state;
      break;
    case 12 :
      cp.digIn1 = bval;
      ++state;
      break;
    case 13 :
      cp.digIn2 = bval;
      ++state;
      break;
    case 14 :
      if (bval == ETX) 
        controlPacketReceived();
      state = 0;
      break;
    }
  }
}

