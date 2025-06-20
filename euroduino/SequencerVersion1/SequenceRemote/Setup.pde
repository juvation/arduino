
void reset()
{
  backDoor = true;

  for (int r=0; r<NUM_ROW; ++r) {
    seq.row[r].noteSlider.setValue(0);
    seq.row[r].assignCB.deactivateAll();
    seq.row[r].markCB.deactivateAll();

    seq.row[r].note = 0;
    for (int i=0; i<NUM_CV-1; ++i)
      seq.row[r].assign[i] = 0;
    for (int i=0; i<NUM_PAD; ++i)
      seq.row[r].mark[i] = 0;
  }

  backDoor = false;
}

// ==================================================================
// create Button, bump xCoord
// ==================================================================
int buttonX, buttonY;

void button(
String name, // button name 
int width, // button width
int id)      // button ID
{
  Button bb = controlP5.addButton(name, 0, buttonX, buttonY, width, 20);
  bb.setId(id);
  buttonX += width + 5;
}

// ==================================================================
// coldstart initialize all widgets, serial port
// ==================================================================

void setup() 
{
  controlP5 = new ControlP5(this);
  mp = new SequencePacket();
  cp = new ControlPacket();
  seq = new Sequence();

  for (int i=0; i<NUM_PAD; ++i)
    cvEntry[i] = new SequencePacket();

  fnt = createFont("Arial", 20, true);
  fnt2 = createFont("Arial", 11, true);
  size(WINDOWXS, WINDOWYS);
  frame.setTitle("Sequencer version 1.0");

  selectSerialPort();

  backDoor = true;

  int yCoord = TOP;
  for (int r=0; r<NUM_ROW; ++r) {
    seq.row[r] = new SequenceRow();
    seq.row[r].noteSlider = controlP5.addSlider(legend[r], 0, MAX_DAC, 0, 10, yCoord, 200, 12);
    seq.row[r].noteSlider.captionLabel().setFont(fnt2);
    seq.row[r].noteSlider.setColorValueLabel(color(255));
    seq.row[r].noteSlider.setColorActive(color(0));

    String name = "\001" + legend[r];
    seq.row[r].assignCB = controlP5.addCheckBox(name);
    seq.row[r].assignCB.setPosition(ASSIGNX, yCoord);
    seq.row[r].assignCB.setColorActive(color(200, 200, 10));
    seq.row[r].assignCB.setSize(15, 15);
    seq.row[r].assignCB.setSpacingColumn(10);
    seq.row[r].assignCB.setItemsPerRow(NUM_PAD);
    for (int c=0; c<NUM_CV; ++c) { 
      String name2 = "\001" + legend[r] + legend[c];
      seq.row[r].assignCB.addItem(name2, 0);
    }

    name = "\002" + legend[r];
    seq.row[r].markCB = controlP5.addCheckBox(name);
    seq.row[r].markCB.setPosition(325, yCoord);
    seq.row[r].markCB.setColorActive(color(10, 230, 10));
    seq.row[r].markCB.setSize(15, 15);
    seq.row[r].markCB.setSpacingColumn(5);
    seq.row[r].markCB.setItemsPerRow(NUM_PAD);
    for (int c=0; c<NUM_PAD; ++c) {
      String name2 = "\002" + legend[r] + legend[c];
      seq.row[r].markCB.addItem(name2, 0);
    }

    yCoord+=18;
    if (r>0 && (r % 5)==4)
      yCoord += 8;
  }

  buttonX = TRIGGERX + 130;
  buttonY = TRIGGERY + 40;
  button(startStopLegend, 90, 1001);

  buttonX = 10;
  buttonY = BOTTOMY+10;
  button("Reset   (S)", 55, 1002);
  button("Random    (R,T)", 70, 1003);
  button(evolveLegend, 65, 1004);

  buttonX = 790;
  buttonY = TRIGGERY + 65;
  button("Save Sequence", 80, 1005);
  button("Load Sequence", 80, 1006);

  glissando = controlP5.addCheckBox("Glissando");
  glissando.setPosition(GLISSX+10, GLISSY+25);
  glissando.setColorActive(color(200, 200, 10));
  glissando.setSize(15, 15);
  glissando.setSpacingColumn(10);
  glissando.setItemsPerRow(NUM_PAD);
  for (int c=0; c<2; ++c) { 
    String name2 = "\001\002" + legend[c];
    glissando.addItem(name2, 0);
  }

  buttonX = GLISSX + 10;
  buttonY = GLISSY + 60;
  button(glissandoWrapLegend, 75, 1007);

  speed = controlP5.addSlider(speedLegend, SPEED_MIN, SPEED_MAX, SPEED_MIN, ASSIGNX, BOTTOMY+11, 115, 18);
  speed.captionLabel().setFont(fnt2);
  speed.setColorLabel(color(0));
  speed.setColorValue(0);
  speed.setColorActive(color(0));
  speed.setColorForeground(color(0));
  speed.setColorValueLabel(color(255));

  changeSpeed(0); // so widget draws current value
  backDoor = false;

  loadFromFile(autoSaveFilename);
  determineCVEntry();
}

// ==================================================================
// extract numeric value characters from file string
// ==================================================================

static String toAscii (
String unicode)  // string from AKWF file 
{
  String output = "";
  char[] charArray = unicode.toCharArray();

  for (int i = 0; i < charArray.length; i++) {
    char a = charArray[i];
    if (a=='-' || (a >='0' && a<='9')) {
      output += a;
    }
  }      
  return output;
}

// ==================================================================

String seqHeader = "OrgoneSequence2 File version 1.0";
final int NUM_FILE_LINES = NUM_ROW + 2; // +2 = header, glissando lines

void loadFromFile(String filename)
{
  String[] data = loadStrings(filename);
  String trim, t1, t2;
  String[] s2;
  int index = 0;

  reset();

  if (data == null) return;
  //println("lf data len = " + data.length);
  if (data.length != NUM_FILE_LINES) 
    return;

  if (!data[0].equals(seqHeader)) 
    return;

  // glissando
  trim = data[1].trim();
  s2 = split(trim, ',');
  for (int i=0; i<NUM_CV-1; ++i) {
    t1 = s2[index++].trim();
    t2 = toAscii(t1);
    glissandoFlag[i] = (byte)int(t2.trim());
  }

  // matrix
  for (int r=0; r<NUM_ROW; ++r) {
    trim = data[r+2].trim();
    s2 = split(trim, ',');

    index = 0;   
    t1 = s2[index++].trim();
    t2 = toAscii(t1);
    seq.row[r].note = (int)int(t2.trim());

    for (int i=0; i<NUM_CV; ++i) {
      t1 = s2[index++].trim();
      t2 = toAscii(t1);
      seq.row[r].assign[i] = (byte)int(t2.trim());
    }

    for (int i=0; i<NUM_PAD; ++i) {
      t1 = s2[index++].trim();
      t2 = toAscii(t1);
      seq.row[r].mark[i] = (byte)int(t2.trim());
    }
  }

  //debug();

  // update widgets to match -----------------------------
  backDoor = true;

  glissando.deactivateAll();
  for (int i=0; i<NUM_CV-1; ++i) {
    if (glissandoFlag[i] != 0) 
      glissando.activate(i);
  }

  for (int r=0; r<NUM_ROW; ++r) {
    seq.row[r].noteSlider.setValue(seq.row[r].note);

    seq.row[r].assignCB.deactivateAll();
    for (int i=0; i<NUM_CV; ++i) {
      if (seq.row[r].assign[i] != 0) {
        seq.row[r].assignCB.activate(i);
      }
    }

    seq.row[r].markCB.deactivateAll();
    for (int i=0; i<NUM_PAD; ++i) {
      if (seq.row[r].mark[i] != 0)
        seq.row[r].markCB.activate(i);
    }
  }

  backDoor = false;
}

// ==================================================================

void saveToFile(String filename)
{
  String[] contents = new String[NUM_FILE_LINES]; 

  int index = 0;
  contents[index++] = seqHeader;

  // glissando line
  contents[index] = "";
  for (int i=0; i<NUM_CV-1; ++i) {
    int v = (int)glissandoFlag[i];
    contents[index] = contents[index] +  nf(v, 1) + ",";
  }
  ++index;

  // matrix lines
  for (int r=0; r<NUM_ROW; ++r) {
    contents[index] = nf(seq.row[r].note, 1) + ",  ";

    for (int i=0; i<NUM_CV; ++i) 
      contents[index] = contents[index] +  nf(seq.row[r].assign[i], 1) + ",";
    contents[index] = contents[index] + "  ";
    for (int i=0; i<NUM_PAD; ++i) 
      contents[index] = contents[index] +  nf(seq.row[r].mark[i], 1) + ",";

    ++index;
  }

  saveStrings(filename, contents);
}

// ==================================================================

void sequenceSelected(File selection)  // selected file info from popup dialog 
{
  if (selection != null)
    loadFromFile(selection.getAbsolutePath());
}

void loadSequence()
{
  selectInput("Select Sequence file", "sequenceSelected");
}

// ==================================================================

void saveSequence()
{
  String fName = showInputDialog("Enter name for this Sequence\n(.seq extension will be added)");
  if (fName == null) return; 
  if ("".equals(fName)) return;
  String wName = fName + ".seq";

  saveToFile(wName);
}

