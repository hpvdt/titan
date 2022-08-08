volatile bool interuptted = false;
bool LEDstate = false;

int interPin = PA0;

void setup() {
  Serial.begin(9600);
  // Setup interrupt pin and interupt routine
  pinMode(interPin, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(interPin), interuptRoutine, RISING);

  // Indicator LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LEDstate);
}

void loop() {
  if (interuptted) {
    Serial.println("hahah");
    interuptted = false;

    // LED blink
    LEDstate = !LEDstate;
    digitalWrite(LED_BUILTIN, LEDstate);
  }
  delay(10);
}

void interuptRoutine() {
  interuptted = true;
}
