#include <TinyGPS.h>
#include "DHT.h" // Used for temperature and humidity

#define FRONTSERIAL Serial1
#define REARSERIAL Serial2
#define GPSSERIAL Serial3
#define DEBUGSERIAL Serial
/* Serial Pins
   PA9 - TX1
   PA10 - RX1

   PA2 - TX2
   PA3 - RX2

   PB10 - TX3
   PB11 - RX3
*/

// DEBUGGING MODE
bool debugMode = true;      // In debug mode or not (copies all messages to debug serial line

const long rpiBaud = 38400;  // Baudrate used with rpis
const long gpsBaud = 9600;    // Baudrate used with GPS
const long debugBaud = 38400;  // Baudrate for debugging line

/////////////////////////////////////////////////
// Variables
int FHR = 10;               // Front heart rate (BPM)
int RHR = 10;               // Rear heart rate
int FCadence = 10;          // Cadence (RPM)
int RCadence = 10;
int FPower = 10;            // Power (W)
int RPower = 10;

int FBatt = 100;            // Front battery %
int RBatt = 100;            // Rear battery %
int SBatt = 100;            // Spare battery %

byte humidity = 0;          // Humidity
byte temperature = 50;      // Temperature

volatile float speedKm = 0.0; // Speed (km/h)
volatile unsigned int rotationCount = 0; // Number of complete wheel roations

float latitude = 0;         // Latitude (degrees)
float longitude = 0;        // Longitude (degrees)
float altitudeGPS = 0;      // Altitude (m)
float speedGPS = 0;         // GPS speed in km/h
float distanceGPS = 0;      // GPS distance in km
// Coordinates that mark the start point (used for getting distance by GPS) 
float startLong = -117.043375;
float startLat = 40.393598;

byte testY = 0;             // Used solely for echo tests
byte testZ = 0;             // Used solely for echo tests

//////////////////////////////////////////////////
// Pin allocation
const byte DHTPin = PB5;  // DHT sensor pin (temperature and humidity)
const byte FBPin = PA0;   // Front battery pin (ADC)
const byte RBPin = PA1;   // Rear battery pin (ADC)
const byte SBPin = PA4;   // Spare battery pin (ADC)
const byte encoderPin = PB8;   // Encoder interrupt pin
const byte S50Pin = PB1;
const byte S25Pin = PB0;

// Digital Humidity and Temperature setup
DHT dht(DHTPin, DHT22); // Sets up sensor
const unsigned int dhtPeriod = 500; // Period between measurements
unsigned long dhtTime = 0; // Stores next time to take measurement

// GPS Module
TinyGPS gps;

// Battery monitoring period
const unsigned long batteryPeriod = 2000; // Battery check period (every few seconds)
unsigned long batteryTime = 0;  // Stores time for next battery check

// Wheel monitoring parameters
const byte numberTicks = 21; // Number of ticks per complete rotation of wheel
volatile byte currentTick = 0; // Stores which tick we are on
volatile byte previousTick = numberTicks; // Stores what tick we were previously on

const float periodToKMH = 7545600.0; //7495200.0; on TITAN // Divide this by period in microseconds to get speed in km/h
// periodToKM is determined by multiplying circumference in meters by 3.6 x 10^6
volatile unsigned long lastPass[numberTicks]; // Stores the last trigger time for each tick
const unsigned long debounce = 4000; // Debounce period in us. Prevents there being repeated measurements from flickering

unsigned long wheelTime = 0; // Check if wheel has timed out (stopped)
const unsigned int wheelTimeout = 500; // Timeout period
float previousSpeed = 0; // Stores speed for comparison


void setup() {
  // Pin modes,
  // !!! verify if encode pin is meant to be pull up or down!!!!!
  pinMode(LED_BUILTIN, OUTPUT); // Used for status indicator
  pinMode(FBPin, INPUT_ANALOG); // Battery pins
  pinMode(RBPin, INPUT_ANALOG);
  pinMode(SBPin, INPUT_ANALOG);
  pinMode(encoderPin, INPUT_PULLUP); // Encoder interupt
  pinMode(S50Pin, OUTPUT);
  pinMode(S25Pin, OUTPUT);

  blinker(2000, 5); // Start-up blinking, gives delay for debugging to connect

  attachInterrupt(encoderPin, encoderDetect, FALLING); // Encoder interupt
  // !!! VERIFY IF FALLING OR RISING !!!

  // Start each serial line
  FRONTSERIAL.begin(rpiBaud);
  REARSERIAL.begin(rpiBaud);
  GPSSERIAL.begin(gpsBaud);

  // Debug timeout
  const int timeout = 2000; // Time out for debugger to be started if debugging
  unsigned long endTime = millis() + timeout; // End time for scanning

  if (debugMode) {
    // Set up debugging mode (USB connection)
    DEBUGSERIAL.begin(debugBaud);

    while (!DEBUGSERIAL) {
      delay(100); // wait for serial port to connect. Needed for native USB

      if (millis() > endTime) {
        // Once timeout has been reached
        debugMode = false; // Disable debugging
        break; // Exit loop
      }
    }

    if (debugMode) {
      DEBUGSERIAL.println(F("DEBUGGING MODE")); // Print debug statement if no timeout
      blinker(600, 3);
    }
  }

  // Sensor setup
  dht.begin(); // Starts digital humidity and temperature sensor

  for (byte i = 0; i < numberTicks; i++) {
    lastPass[i] = 0; // Reset all encoder tick values
  }

  blinker(100, 10); // Blink a few times on successful startup
}

void loop() {
  if (FRONTSERIAL.available()) {
    // Data on front line
    processData('f');
  }
  if (REARSERIAL.available()) {
    // Data on rear line
    processData('r');
  }
  if (DEBUGSERIAL.available()) {
    // Data on debugging line
    processData('d');
  }

  // Periodic DHT measurement
  if (millis() > dhtTime) {
    humidity = dht.readHumidity() * 2;            // Humidity reading
    temperature = 50 + dht.readTemperature() * 2; // Temperature reading

    dhtTime = millis() + dhtPeriod; // Sets next measurement time
  }

  // Periodic battery check
  if (millis() > batteryTime) {
    FBatt = batteryLevel('f');
    RBatt = batteryLevel('r');
    SBatt = batteryLevel('s');

    batteryTime = millis() + batteryPeriod; // Sets next check
  }

  // Periodic speed check, to see if we stopped
  if (millis() > wheelTime) {
    if (speedKm == previousSpeed) speedKm = 0; // Reset speed if no change

    previousSpeed = speedKm; // Updates speed
    wheelTime = millis() + wheelTimeout; // Sets next check
  }

  /////////////////////////////////////////
  // GPS code
  if (GPSSERIAL.available()) {
    GPSCheck();
  }
  delayMicroseconds(500); // Regular delay
  digitalWrite(LED_BUILTIN, digitalRead(encoderPin));
}

void blinker (const int period, const byte repeats) {
  // Blink LED a number of times for a given length
  for (byte i = 0; i < repeats; i++) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(period / 2);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(period / 2);
  }
  // Note on STM32 LED is on when pin is LOW!
}
