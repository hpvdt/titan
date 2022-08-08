const int hallpin = 3; // Hall effect line (interupt)

unsigned long lasttime;
volatile unsigned long newtime; // Used to keep track of wheel rotational period
double circumference = 2070.0; // Circumference of the wheel in m (x 1000)


void setup() {
  Serial.begin(9600);
  
  // Sets values
  lasttime = millis();
  newtime = lasttime;
  
  pinMode(3,INPUT_PULLUP); // A3144 works by pulling down its output pin in the presence of a field 
  attachInterrupt(digitalPinToInterrupt(hallpin), updateTime, FALLING); // Hall interupt
}

void loop() {
  delay(10);
  if (newtime != lasttime) {
    // Update speed
    unsigned long period = newtime - lasttime;
    lasttime = newtime;
    
    Serial.println(circumference / period); // Outputs m/s
    // Uses the fact that circ is 1000 times as big than it should be to get m/s instead of m/ms
  }
}
