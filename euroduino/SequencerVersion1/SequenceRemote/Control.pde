Boolean handleEvent(int id)
{
  boolean needToSave = false;

  switch(id) { // Button IDs
  case 1001 :
    isPlaying = !isPlaying;
    controlP5.controller(startStopLegend).setColorBackground(isPlaying ? HIGHLIGHTCOLOR : BLUECOLOR);
    playIndex = 0;
    break;
  case 1002 :
    reset();
    needToSave = true;
    break;
  case 1003 :
    randomize();
    needToSave = true;
    break;
  case 3000 :
    randomize2();
    needToSave = true;
    break;
  case 1004 :
    evolveFlag = !evolveFlag;
    controlP5.controller(evolveLegend).setColorBackground(evolveFlag ? HIGHLIGHTCOLOR : BLUECOLOR);
    break;
  case 1005 :
    saveSequence();
    break;
  case 1006 :
    loadSequence();
    break;
  case 1007 :
    glissandoWrapFlag = !glissandoWrapFlag;
    controlP5.controller(glissandoWrapLegend).setColorBackground(glissandoWrapFlag ? HIGHLIGHTCOLOR : BLUECOLOR);
    determineCVEntry();
    break;
  }

  return needToSave;
}

// ==================================================================

void controlEvent(ControlEvent theEvent) 
{
  if (backDoor) return; // during file load, do not alter storage

  boolean needToSave = false;

  if (theEvent.isController()) { 
    if (theEvent.controller().name()== speedLegend)
      playSpeed = (int)theEvent.controller().value();
  } else {    
    // assignment checkboxes ------------------------------
    for (int r=0; r<NUM_ROW; ++r) {
      if (theEvent.isFrom(seq.row[r].assignCB)) {
        needToSave = true;
        for (int m=0; m<NUM_CV; ++m) {
          seq.row[r].assign[m] = 0; // assume not assigned
          if (seq.row[r].assignCB.getArrayValue()[m] > 0) {
            seq.row[r].assign[m] = 1;
          }
        }
      }
    }

    // glissando checkboxes -------------------------------
    if (theEvent.isFrom(glissando)) {
      for (int m=0; m<2; ++m) {
        glissandoFlag[m] = 0; // assume not checked
        if (glissando.getArrayValue()[m] > 0) {
          glissandoFlag[m] = 1;
        }
      }

      determineCVEntry();
    }
  }

  // matrix buttons -
  for (int r=0; r<NUM_ROW; ++r) {
    if (theEvent.isFrom(seq.row[r].noteSlider)) {
      seq.row[r].note = (int)seq.row[r].noteSlider.getValue();
      needToSave = true;
    }

    if (theEvent.isFrom(seq.row[r].markCB)) {
      needToSave = true;
      for (int c=0; c<NUM_PAD; ++c) 
        seq.row[r].mark[c] = (byte)seq.row[r].markCB.getArrayValue()[c];
    }
  }

  if (needToSave) {
    saveToFile(autoSaveFilename);
    determineCVEntry();
  }
}

// ==================================================================

void debug()
{
  println("isPlaying = " + isPlaying);
  for (int r=0; r<NUM_ROW; ++r) {
    print("row:" + r + " note:" + seq.row[r].note + " Assign: ");

    for (int i=0; i<NUM_CV; ++i)
      print(seq.row[r].assign[i] + ",");

    print(" Pad: ");
    for (int i=0; i<NUM_PAD; ++i)
      print(seq.row[r].mark[i] + ",");

    println();
  }
}

// ==================================================================

Boolean seldom()
{
  return random(0, 100) >= 95;
}

void randomize() // add random entries
{
  for (int r=0; r<NUM_ROW; ++r) {
    seq.row[r].note = (int)random(0, MAX_DAC-20);
    seq.row[r].noteSlider.setValue(seq.row[r].note);

    for (int i=0; i<NUM_CV; ++i) {
      if (seldom()) {
        seq.row[r].assign[i] = 1;
        seq.row[r].assignCB.activate(i);
      }
    }

    for (int i=0; i<NUM_PAD; ++i) {
      if (seldom()) {
        seq.row[r].mark[i] = 1;
        seq.row[r].markCB.activate(i);
      }
    }
  }
}

void randomize2() // remove random entries
{
  for (int r=0; r<NUM_ROW; ++r) {
    for (int i=0; i<NUM_CV; ++i) {
      if (seldom()) {
        seq.row[r].assign[i] = 0;
        seq.row[r].assignCB.deactivate(i);
      }
    }

    for (int i=0; i<NUM_PAD; ++i) {
      if (seldom()) {
        seq.row[r].mark[i] = 0;
        seq.row[r].markCB.deactivate(i);
      }
    }
  }
}

// ==============================================================

void controlPacketReceived()
{
  if(cp.digIn1 != 0) 
    receivedTrigger();
    
  //println("P1: " + cp.pot1 + " P2:" + cp.pot2 + " Cv1: " + cp.cvIn1 + " Cv2:" + cp.cvIn2 + " S1:" + cp.sw1 + " S2:" + cp.sw2 + " D1:" + cp.digIn1 + " D2:" + cp.digIn2);
}

