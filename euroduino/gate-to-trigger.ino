// These constants won't change.  They're used to give names
// to the pins used:
const int analogIn1Pin = A0;  // Analog Input 1
const int analogIn2Pin = A1; // Analog Input 2
const int analogPot1Pin = A2;  // Pot 1
const int analogPot2Pin = A3; // Pot 2
const int analogOut1Pin = 5;  // Analog Output 1
const int analogOut2Pin = 6; // Analog Output 2
const int DigitalIn1Pin = 8;  // Digital Input 1
const int DigitalIn2Pin = 9;  // Digital Input 2
const int DigitalOut1Pin = 3;  // Digital Output 1
const int DigitalOut2Pin = 4;  // Digital Output 2
const int Switch1Up = A4;  // Switch 1 Up
const int Switch1Dwn = A5;  // Switch 1 Dwn
const int Switch2Up = 7;  // Switch 2 Up
const int Switch2Dwn = 2;  // Switch 2 Dwn

bool gate1;
bool gate2;
unsigned char digitalIn1;
unsigned char digitalIn2;

void setup() {
  /* Set up I/O pins */
  pinMode(DigitalIn1Pin, INPUT);
  pinMode(DigitalIn2Pin, INPUT);
  pinMode(DigitalOut1Pin, OUTPUT); 
  pinMode(DigitalOut2Pin, OUTPUT); 
  pinMode(Switch1Up, INPUT_PULLUP);
  pinMode(Switch1Dwn, INPUT_PULLUP);
  pinMode(Switch2Up, INPUT_PULLUP);
  pinMode(Switch2Dwn, INPUT_PULLUP);
  
  gate1 = false;
  gate2 = false;
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalIn1 = digitalRead(DigitalIn1Pin);
  digitalIn2 = digitalRead(DigitalIn2Pin);
  
  if(digitalIn1 == 1 && gate1 == false) {
    // gate is on, send trig
    gate1 = true;
    send_trig();
  }
  if(digitalIn2 == 1 && gate2 == false && gate1 == true) {
    gate2 = true;
    send_trig();
  }
  if (digitalIn1 == 0) {
    // gate is off
    gate1 = false;
  }
  if (digitalIn2 == 0) {
    gate2 = false;
  }
}

void send_trig() {
  digitalWrite(DigitalOut1Pin, 1);
  delay(1);
  digitalWrite(DigitalOut1Pin, 0);
}