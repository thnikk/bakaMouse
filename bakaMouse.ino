#include <Arduino.h>

// Inlcude necessary libraries
#include "Mouse.h"
#include <math.h>

// Button mapping
#define m1p MOUSE_LEFT
#define m2p MOUSE_MIDDLE
#define m3p MOUSE_RIGHT

// Button pins
const int m1 = 2;
const int m2 = 4;
const int m3 = 6;

// Thumbstick pins
const int xAxis = A0;         // joystick X axis
const int yAxis = A1;         // joystick Y axis

// parameters for reading the joystick:
int range = 64;               // output range of X or Y movement
int center = range/2;       // resting position value

// XY radius
int r = 0;
int deadzone = 1;

// Polling rate
int poll = 10;

unsigned long prevMillis = 0;

void setup() {

  // Setup for inputs
  pinMode(m1, INPUT_PULLUP);
  pinMode(m2, INPUT_PULLUP);
  pinMode(m3, INPUT_PULLUP);

  // Setup for mouse library
  Mouse.begin();
}

void loop() {

  unsigned long curMillis = millis();

  // read and scale the two axes:
  int xReading = readAxis(xAxis, 1);
  int yReading = readAxis(yAxis, 0);

  r = sqrt(sq(xReading) + sq(yReading));

  if (r <= deadzone) {
    xReading = 0;
    yReading = 0;
  }

  if (curMillis - prevMillis > poll) {
    Mouse.move(xReading, yReading, 0);

    // For debuging button inputs
    serialDebug(0, 1);

    prevMillis = curMillis;
  }


  //delay(4);

  buttonPresses();
}

void buttonPresses() {

  if (digitalRead(m1) == LOW) {
    Mouse.press(m1p);
  }
  if (digitalRead(m1) == HIGH) {
    Mouse.release(m1p);
  }
  /*
  if (digitalRead(m2) == LOW) {
   Mouse.press(m2p);
  }
  if (digitalRead(m2) == HIGH) {
   Mouse.release(m2p);
  }
  */
  if (digitalRead(m3) == LOW) {
    Mouse.press(m3p);
  }
  if (digitalRead(m3) == HIGH) {
    Mouse.release(m3p);
  }
}

/* reads an axis (0 or 1 for x or y) and scales the
analog input range to a range from 0 to <range> */

int readAxis(int thisAxis, int invert) {
  // read the analog input:
  int reading = analogRead(thisAxis);

  if (invert == 1) {
    reading = ((reading * -1)+ 1024);
  }

  // map the reading from the analog input range to the output range:
  reading = map(reading, 0, 1023, 0, range);

  // if the output reading is outside from the
  // rest position threshold,  use it:
  int distance = reading - center;

  if (distance > 0) {
    distance = fscale(0, 32, 0, 32, distance, -1);
  }
  if (distance < 0) {
    // Make distance positive
    distance = distance * -1;

    distance = fscale(0, 32, 0, 32, distance, -1);
    // Make it negative again

    distance = distance * -1;
  }

  // return the distance for this axis:
  return distance;
}

void serialDebug(int button, int radius) {

  if (button == 1) {
    Serial.print("(");
    Serial.print(digitalRead(m1));
    Serial.print(", ");
    Serial.print(digitalRead(m2));
    Serial.print(", ");
    Serial.print(digitalRead(m3));
    Serial.println(")");
  }

  if (radius == 1) {
    Serial.print("(");
    Serial.print(r);
    Serial.println(")");
  }
}

float fscale( float originalMin, float originalMax, float newBegin, float
newEnd, float inputValue, float curve){

  float OriginalRange = 0;
  float NewRange = 0;
  float zeroRefCurVal = 0;
  float normalizedCurVal = 0;
  float rangedValue = 0;
  boolean invFlag = 0;


  // condition curve parameter
  // limit range

  if (curve > 10) curve = 10;
  if (curve < -10) curve = -10;

  curve = (curve * -.1) ; // - invert and scale - this seems more intuitive - postive numbers give more weight to high end on output
  curve = pow(10, curve); // convert linear scale into lograthimic exponent for other pow function

  /*
   Serial.println(curve * 100, DEC);   // multply by 100 to preserve resolution
   Serial.println();
   */

  // Check for out of range inputValues
  if (inputValue < originalMin) {
    inputValue = originalMin;
  }
  if (inputValue > originalMax) {
    inputValue = originalMax;
  }

  // Zero Refference the values
  OriginalRange = originalMax - originalMin;

  if (newEnd > newBegin){
    NewRange = newEnd - newBegin;
  }
  else
  {
    NewRange = newBegin - newEnd;
    invFlag = 1;
  }

  zeroRefCurVal = inputValue - originalMin;
  normalizedCurVal  =  zeroRefCurVal / OriginalRange;   // normalize to 0 - 1 float

  /*
  Serial.print(OriginalRange, DEC);
   Serial.print("   ");
   Serial.print(NewRange, DEC);
   Serial.print("   ");
   Serial.println(zeroRefCurVal, DEC);
   Serial.println();
   */

  // Check for originalMin > originalMax  - the math for all other cases i.e. negative numbers seems to work out fine
  if (originalMin > originalMax ) {
    return 0;
  }

  if (invFlag == 0){
    rangedValue =  (pow(normalizedCurVal, curve) * NewRange) + newBegin;

  }
  else     // invert the ranges
  {
    rangedValue =  newBegin - (pow(normalizedCurVal, curve) * NewRange);
  }

  return rangedValue;
}
