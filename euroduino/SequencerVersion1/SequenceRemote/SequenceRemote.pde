// Sequencer version1
//
// be sure to download the companion EuroDuinoA code
//
// compile problem:  "No library found for controlP5"  
//  1. select Menu option <Sketch><Import Library><Add Library...>
//  2. type ControlP5 into edit field to narrow the search
//  3. select and install ControlP5 by Andreas Schiegel
// ====================================================================================================

import processing.serial.*;
import controlP5.*;
import static javax.swing.JOptionPane.*;
import java.awt.event.MouseEvent.*;
import java.util.*;

final int NUM_CV = 4;    // # output controls:   cv1,cv2,dig1,dig2
final int NUM_PAD = 32;
final int NUM_ROW = 20;
final int SPEED_MIN = 1;
final int SPEED_MAX = 400;
final int TOPY = 18;
final int TOP = 25;
final int WINDOWXS = 975;
final int WINDOWYS = 542;
final int BOTTOMY = 5+NUM_ROW*20;
final int ASSIGNX = 223;
final int TRIGGERX = 10;
final int TRIGGERY = BOTTOMY+40;
final int TRIGGERXS = 250;
final int TRIGGERYS = 85;
final int GLISSX = TRIGGERX+TRIGGERXS+10;
final int GLISSY = BOTTOMY+40;
final int GLISSXS = 118;
final int GLISSYS = TRIGGERYS;
final int CVPANELX = GLISSX+GLISSXS+10;
final int CVPANELY = BOTTOMY+40;
final int CVPANELXS = 250;
final int CVPANELYS = TRIGGERYS;
final int MAX_DAC = 255;

final byte UNUSED = (byte)255;
final color BLUECOLOR = color(19, 52, 82);
final color HIGHLIGHTCOLOR = color(138, 16, 24);  

class SequencePacket { 
  byte  stx;
  byte  typecode;
  byte[]  data = new byte[NUM_CV];    // cvOut1,cvOut2,digOut1,digOut2
  byte  etx;
}

class ControlPacket { 
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
}

class SequenceRow {
  Slider noteSlider;
  CheckBox markCB;
  CheckBox assignCB;
  int note;
  byte[] assign = new byte[NUM_CV];
  byte[] mark = new byte[NUM_PAD];
}

class Sequence { // matrix of sequencer nodes
  SequenceRow[] row = new SequenceRow[NUM_ROW];
}

SequencePacket[] cvEntry = new SequencePacket[NUM_PAD];
SequencePacket temp = new SequencePacket(); // calculated values between matrix nodes
SequencePacket mp;
ControlPacket cp;

Sequence seq;
ControlP5 controlP5;
Serial port;
PFont fnt, fnt2;         
Button startStop;
Slider speed;
RadioButton[] assign = new RadioButton[NUM_CV];
CheckBox glissando;
Button glissandoWrap;
boolean glissandoWrapFlag = false;
boolean triggeredPlay = false;
boolean isPlaying = false;
boolean backDoor = false;  // set when you want to update widgets without side effects
boolean evolveFlag = false;
byte[] glissandoFlag = new byte[NUM_CV-1];
int playIndex;
int playSpeed = 4;

final String glissandoWrapLegend = "WrapAround";
final String autoSaveFilename = "EuroDuinoSequence.seq"; 
final String startStopLegend = "Start/Stop  (Spc)";
final String evolveLegend = "Evolve   (E)";
final String speedLegend = "Speed  (1,2)";
final String[] checkBoxLegend = { 
  "C1", "C2", "D1", "D2"
};
final String checkBoxLegend2 = "(C1: CV Out #1,  C2: CV Out #2,   D1: Dig Out #1,  D2: Dig Out #2)";

// Processing requires a unique name for every widget.  These names do not display on screen.
final String[] legend = {
  "\001\001\001", "\001\001\002", "\001\001\003", "\001\001\004", "\001\002\001", "\001\002\002", "\001\002\003", "\001\002\004", 
  "\001\003\001", "\001\003\002", "\001\003\003", "\001\003\004", "\001\004\001", "\001\004\002", "\001\004\003", "\001\004\004", 
  "\002\001\001", "\002\001\002", "\002\001\003", "\002\001\004", "\002\002\001", "\002\002\002", "\002\002\003", "\002\002\004", 
  "\002\003\001", "\002\003\002", "\002\003\003", "\002\003\004", "\002\004\001", "\002\004\002", "\002\004\003", "\002\004\004", 
  "\003\001\001", "\003\001\002", "\003\001\003", "\003\001\004", "\003\002\001", "\003\002\002", "\003\002\003", "\003\002\004", 
  "\003\003\001", "\003\003\002", "\003\003\003", "\003\003\004", "\003\004\001", "\003\004\002", "\003\004\003", "\003\004\004", 
  "\004\001\001", "\004\001\002", "\004\001\003", "\004\001\004", "\004\002\001", "\004\002\002", "\004\002\003", "\004\002\004", 
  "\004\003\001", "\004\003\002", "\004\003\003", "\004\003\004", "\004\004\001", "\004\004\002", "\004\004\003", "\004\004\004",
};

// ==================================================================

int byteToInt(byte b)   // can't trust Processing
{ 
  int v = (int)b;
  if (v < 0) v += 256;
  return v;
}

// ==================================================================

int playPace;

void draw() 
{ 
  background(100);

  fill(0);
  textFont(fnt, 15);
  text("CV input", 10, TOPY);
  for (int i=0; i<NUM_CV; ++i)
    text(checkBoxLegend[i], ASSIGNX+i*25, TOPY);
  text("Sequencer Markers. Check columns where note should be sounded.", ASSIGNX+170, TOPY);
  text(checkBoxLegend2, ASSIGNX+250, BOTTOMY+25);

  drawExternalTrigger();
  drawGlissandoPanel();

  if (isPlaying || triggeredPlay) {
    if (++playPace >= playSpeed) {
      playPace = 0;
      if (++playIndex >= NUM_PAD) {
        playIndex=0 ;
        triggeredPlay = false;
      }

      if (isPlaying)
        playPad();
    } else 
      interpolatePlayPad();

    if (evolveFlag) {
      if (random(1, 20) < 3) {
        if (random(1, 20) < 3) {
          int r = (int)random(0, NUM_ROW-1);
          int n = (int)random(0, MAX_DAC-20);
          seq.row[r].noteSlider.setValue(n);
        }
        if (random(1, 20) < 3) {
          int r = (int)random(0, NUM_ROW-1);
          int n = (int)random(0, NUM_CV);
          if (random(0, 1) < 0.5)
            seq.row[r].assignCB.activate(n);
          else
            seq.row[r].assignCB.deactivate(n);
        }
        if (random(1, 20) < 3) {
          int r = (int)random(0, NUM_ROW-1);
          int n = (int)random(0, NUM_PAD);
          if (random(0, 1) < 0.5)
            seq.row[r].markCB.activate(n);
          else
            seq.row[r].markCB.deactivate(n);
        }
      }
    }

    drawPlayIndex();
  }

  drawCVPanel();
  monitorSerialPort();
}

//==================================================================

int triggerReceivedDisplayCount = 0;

final int ET_X = TRIGGERX + 10;
final int ET_Y = TRIGGERY + 14;

void drawExternalTrigger()
{
  fill(90);
  stroke(0);
  rect(TRIGGERX, TRIGGERY, TRIGGERXS, TRIGGERYS);

  fill(triggerReceivedDisplayCount > 0 ? color(255, 0, 0) : color(80));
  rect(ET_X, ET_Y, 11, 11);

  fill(0);
  text("External Trigger from Dig In#1", TRIGGERX+30, TRIGGERY+25);
  text("Continuous Run", TRIGGERX+10, TRIGGERY+55);

  if (triggerReceivedDisplayCount > 0)
    --triggerReceivedDisplayCount;
}

// ==================================================================

void drawPlayIndex()
{
  int x1 = 329;
  int y1 = BOTTOMY+2;

  fill(100);
  stroke(100);
  rect(x1, y1, 630, 7);

  fill(255);
  x1 += playIndex*20;
  rect(x1, y1, 7, 7);
}

// ==================================================================

void drawGlissandoPanel()
{
  fill(90);
  stroke(0);
  rect(GLISSX, GLISSY, GLISSXS, GLISSYS);

  fill(0);
  text("Glissando", GLISSX+10, GLISSY+20);
  for (int i=0; i<2; ++i)
    text(checkBoxLegend[i], GLISSX+10+i*25, GLISSY+54);
}

// ==================================================================

final int CVBARX = CVPANELX+35;
final int CVBARXS = 200;
final int CVBARYS = 8;

void drawCVBar(int y, int value)
{
  int x = CVBARXS * value / MAX_DAC;
  stroke(0);

  fill(30, 144, 255);
  rect(CVBARX, y, x, CVBARYS);

  fill(80);
  rect(CVBARX+x, y, CVBARXS-x, CVBARYS);
}

void drawCVBar2(int y, byte onoff)
{
  stroke(0);

  if (onoff != 0)
    fill(30, 144, 255);
  else
    fill(80);

  rect(CVBARX, y, CVBARYS, CVBARYS);
}

void drawCVPanel()
{
  fill(90);
  stroke(0);
  rect(CVPANELX, CVPANELY, CVPANELXS, CVPANELYS);

  fill(0);
  for (int i=0; i<NUM_CV; ++i)
    text(checkBoxLegend[i], CVPANELX+10, CVPANELY+17+i*16);

  drawCVBar(CVPANELY+7+0*16, byteToInt(temp.data[0]));
  drawCVBar(CVPANELY+7+1*16, byteToInt(temp.data[1]));
  drawCVBar2(CVPANELY+7+2*16, temp.data[2]);
  drawCVBar2(CVPANELY+7+3*16, temp.data[3]);
}

// ==================================================================

void changeSpeed(int dir) 
{
  int amt = 10;
  if (playSpeed <= 25) amt = 5;
  if (playSpeed <= 10) amt = 1;

  playSpeed = constrain(playSpeed + dir * amt, SPEED_MIN, SPEED_MAX);
  if (speed != null)
    speed.setValue(playSpeed);
}

// ==================================================================

void keyPressed() 
{
  int ch = key;
  if (ch > 'Z') ch -= ('a' - 'A');

  switch(ch) {
  case '1' : 
    changeSpeed(-1); 
    break;
  case '2' : 
    changeSpeed(+1); 
    break;
  case ' ' :  // start/stop
    handleEvent(1001);
    break;
  case 'S' :  // reset
    handleEvent(1002);
    break;
  case 'R' :  // random+
    handleEvent(1003);
    break;
  case 'T' :  // random-
    handleEvent(3000);
    break;
  case 'E' :  // evolve
    handleEvent(1004);
    break;

  case '8' :
    cvEntryDebug();
    break;
  case '9' :
    debug();
    break;
  }
}

// ==================================================================
// play matrix entry when first encountered during playback

void playPad()
{
  temp = cvEntry[playIndex];
  sendSequencePacket(temp);
}

// ==================================================================
// interpolate a matrix entry when we are stalling between matrix entries 

void interpolatePlayPad()
{
  sendLowGate();

  int index2 = playIndex+1;
  if (index2 >= NUM_PAD) index2 = 0;

  float ratio = (float)playPace / (float)playSpeed;

  for (int i=0; i<NUM_CV; ++i) 
    temp.data[i] = cvEntry[playIndex].data[i];

  for (int i=0; i<2; ++i) {  // glissando only applies to CV ports
    boolean applyGlissando = (glissandoFlag[i] > 0);
    if (applyGlissando && index2 == 0 && !glissandoWrapFlag)
      applyGlissando = false;

    if (applyGlissando) {
      int v1 = byteToInt(cvEntry[playIndex].data[i]);
      int v2 = byteToInt(cvEntry[index2   ].data[i]);
      temp.data[i] = (byte)((float)v1 + (float)(v2-v1) * ratio);
    }
  }

  sendSequencePacket(temp);
}

// ==================================================================
// copy current glissando value to end of matrix

void glissandoToEnd(int cvIndex, int i1)
{
  if (i1 == -1) return;

  for (int i=i1+1; i<NUM_PAD; ++i) 
    cvEntry[i].data[cvIndex] = cvEntry[i1].data[cvIndex];
}

// ==================================================================
// given two matrix entry indices, fill interveening entries with linear interpolated values

void glissando(int cvIndex, int i1, int i2)
{
  int iDiff = i2-i1;
  if (iDiff < 2) return;
  // println("CV:" + cvIndex + " i1: " + i1 + " i2:" + i2);

  // glissando disabled. copy current cv value to succeeding matrix columns
  if (glissandoFlag[cvIndex] == 0) {
    for (int i=i1+1; i<i2; ++i) 
      cvEntry[i].data[cvIndex] = cvEntry[i1].data[cvIndex];
  } else {
    int vDiff = byteToInt(cvEntry[i2].data[cvIndex]) - byteToInt(cvEntry[i1].data[cvIndex]);
    float vHop = (float)vDiff / (float)iDiff;
    float value = (float)cvEntry[i1].data[cvIndex];

    for (int i=i1+1; i<i2; ++i) {
      value += vHop;
      cvEntry[i].data[cvIndex] = (byte)value;
    }
  }
}

// ==================================================================
// wrap around from last note back to first

void glissandoWrapAround(int cvIndex, int i1, int i2) // i1 is higher than i2
{
  if (i1 == -1) return;
  if (i2 == -1) return;
  if (i1 >= NUM_PAD) i1 = NUM_PAD-1;

  int iDiff = (NUM_PAD - i1) + i2;
  if (iDiff < 2) return;

  int vDiff = byteToInt(cvEntry[i2].data[cvIndex]) - byteToInt(cvEntry[i1].data[cvIndex]);
  float vHop = (float)vDiff / (float)iDiff;
  float value = (float)cvEntry[i1].data[cvIndex];

  for (int i=1; i<iDiff; ++i) {
    value += vHop;
    int index = i1+i;
    if (index >= NUM_PAD) index -= NUM_PAD;
    cvEntry[index].data[cvIndex] = (byte)value;
  }
}

// ==================================================================
// copy note entries to succeeding empty columns

void propagateCV(int index)
{
  byte v = cvEntry[0].data[index];

  for (int i=1; i<NUM_PAD; ++i) {
    if (cvEntry[i].data[index] == 0)
      cvEntry[i].data[index] = v;
    else 
      v = cvEntry[i].data[index];
  }
}

// ==================================================================
// walk all of note matrix to determine final CV and Gate values for each column

void determineCVEntry()
{
  for (int p=0; p<NUM_PAD; ++p) {
    for (int m=0; m<2; ++m) {        // CV ports
      cvEntry[p].data[m] = 0;

      for (int r=0; r<NUM_ROW; ++r) {
        if ((seq.row[r].mark[p] > 0) && (seq.row[r].assign[m] > 0)) 
          cvEntry[p].data[m] = (byte)seq.row[r].note;
      }
    }

    for (int m=2; m<4; ++m) {        // digital ports
      cvEntry[p].data[m] = 0;

      for (int r=0; r<NUM_ROW; ++r) {
        if ((seq.row[r].mark[p] > 0) && (seq.row[r].assign[m] > 0)) 
          cvEntry[p].data[m] = 1;
      }
    }
  }

  // CV ports propagate value to succeeding columns --
  for (int m=0; m<2; ++m) {     

    if (glissandoFlag[m] == 0) 
      propagateCV(m);
    else {
      int iFirst=-1, iLast=-1, i1, i2;
      i1 = 0;
      i2 = 0;
      iFirst = -1;
      iLast = -1;

      for (;; ) {
        // find 1st entry
        for (; i1<NUM_PAD; ++i1)
          if (byteToInt(cvEntry[i1].data[m]) > 0) break;

        if (i1 == NUM_PAD) break;  // none
        if (iFirst == -1) iFirst = i1;

        // find 2nd entry
        for (i2=i1+1; i2<NUM_PAD; ++i2)
          if (byteToInt(cvEntry[i2].data[m]) > 0) break;

        if (i2 != NUM_PAD) {
          iLast = i2;
          glissando(m, i1, i2);
          i1 = i2;
        } else
          break;  // no 2nd entry
      }

      if (glissandoWrapFlag)
        glissandoWrapAround(m, iLast, iFirst);
      else {
        if (i2 >= NUM_PAD)
          glissandoToEnd(m, i1);
      }
    }
  }

  //cvEntryDebug();
}

// ==================================================================

void cvEntryDebug()
{
  println();
  for (int m=0; m<NUM_CV; ++m) {
    print("CV" + m + ": ");
    for (int p=0; p<NUM_PAD; ++p) 
      print(byteToInt(cvEntry[p].data[m]) + ", ");
    println();
  }
}

// ==================================================================

void receivedTrigger()
{
  triggeredPlay = true;
  playIndex = 0;
  playPace = 0;
  triggerReceivedDisplayCount = 8;
}

