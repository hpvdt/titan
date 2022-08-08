char request; // Used to store instruction
String temp;  // Used to store temp string

void setup() {
  Serial1.begin(9600);
}

void loop() {
  if (Serial1.available() > 0) {
    request = Serial1.read();
    switch (request) {
      case 's':
        blinker(500);
        temp = "Speed is :";
        temp += String(random(30));
        sendMessage(temp); // Sends whatever message is needed
        break;
      case 't':
        blinker(200);
        temp = "Temperature is ";
        temp += String(random(50));
        sendMessage(temp); // Sends whatever message is needed
        break;
    }

  }
  delay(10);
}

void sendMessage (String message) {
  // Sends message in data frame
  byte messageLength = 31 + message.length(); // Shifts it to be a printable character
  Serial1.write(messageLength); // Output length
  Serial1.print(message); // Prints message itself
}

void blinker( int timein) {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(timein);
  digitalWrite(LED_BUILTIN, LOW);
  delay(timein);
}
