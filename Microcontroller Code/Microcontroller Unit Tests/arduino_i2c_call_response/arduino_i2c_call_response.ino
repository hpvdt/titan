#include <Wire.h>

// A4 is SDA
// A5 is SCL

char request; // Stores requested data to relay

void setup() {
  Wire.begin(8);                // join i2c bus with address #8
  Wire.onReceive(receiveEvent); // register event
  Wire.onRequest(requestEvent);

  // This program works by first recieveing the type of data the rpi wants from it as a byte
  // Then when the rpi sends a request we reply with the appropriate data

  pinMode(LED_BUILTIN, OUTPUT); // LED is used for status
}

void loop() {
  delay(10);
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany) {
  char request = Wire.read(); // receives request byte as a character

  if (request == 't') blinker(250);
}

void requestEvent() {
  //  String temp = "";
  //
  //  switch (request) {
  //    case 's':
  //      temp = "Speed is :";
  //      temp += String(random(30));
  //      break;
  //    case 't':
  //      temp = "Temperature is ";
  //      temp += String(random(50));
  //      break;
  //  }
  //
  //  if (temp != "") {
  //    byte messageLength = 31 + temp.length(); // Shifts it to be a printable character
  //    Wire.write(messageLength); // Output length
  //    Wire.write(temp.c_str(), messageLength - 1); // Sends message itself, excluding the null termination
  //  }
  Wire.write(255);
  
  blinker(500);
  blinker(500);
}

void blinker( int timein) {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(timein);
  digitalWrite(LED_BUILTIN, LOW);
  delay(timein);
}
