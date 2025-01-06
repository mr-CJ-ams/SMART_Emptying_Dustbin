#define BLYNK_TEMPLATE_ID "TMPL642330v8l"
#define BLYNK_TEMPLATE_NAME "SMART DUSTBIN"
#define BLYNK_AUTH_TOKEN "kJbeOeBxoxmliVko4JaGbAgF4rQlm8ZQ"

#include <TinyGsmClient.h>
#include <BlynkSimpleTinyGSM.h>

// Define your GSM module's serial interface pins
#define RX_PIN 13 // SIM800L TX -> ESP32 RX
#define TX_PIN 17 // SIM800L RX -> ESP32 TX

// Define your GSM/GPRS credentials
const char apn[] = "http.globe.com.ph"; // Globe APN
const char user[] = ""; // No username required for Globe
const char pass[] = ""; // No password required for Globe

// Setup hardware serial for SIM800L
HardwareSerial SIM800L(2);

// Setup TinyGSM client
TinyGsm modem(SIM800L);
BlynkTimer timer;

// Define pins for Ultrasonic Sensors and LEDs
const int trigPins[] = {5, 6, 7}; // Trigger pins
const int echoPins[] = {18, 19, 12}; // Echo pins
const int ledPins[] = {4, 10, 3}; // LED pins
const int virtualPins[] = {V3, V4, V5}; // Blynk virtual pins

// State variables
float distances[3];
unsigned long timers[3] = {0, 0, 0};
bool ledStates[3] = {false, false, false};

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(115200);
  SIM800L.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);

  // Initialize pin modes for sensors and LEDs
  for (int i = 0; i < 3; i++) {
    pinMode(trigPins[i], OUTPUT);
    pinMode(echoPins[i], INPUT);
    pinMode(ledPins[i], OUTPUT);
  }

  // Initialize GSM module
  Serial.println("Initializing GSM...");
  if (!modem.restart()) {
    Serial.println("Failed to restart modem");
    while (true);
  }

  // Connect to the GPRS network
  Serial.print("Connecting to GPRS...");
  if (!modem.gprsConnect(apn, user, pass)) {
    Serial.println("Failed to connect to GPRS");
    while (true);
  }
  Serial.println("GPRS connected");

  // Initialize Blynk with GSM
  Blynk.begin(BLYNK_AUTH_TOKEN, modem, apn, user, pass);

  Serial.println("Setup complete. Sensors are ready.");
}

void loop() {
  Blynk.run(); // Run Blynk

  // Measure distances and handle sensors
  for (int i = 0; i < 3; i++) {
    distances[i] = measureDistance(trigPins[i], echoPins[i]);
    handleSensor(i);
  }

  delay(50); // Small delay for stability
}

// Function to measure distance using Ultrasonic Sensor
float measureDistance(int trigPin, int echoPin) {
  long duration;
  float distance;

  // Send a 10us pulse to trigger pin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Measure the duration of the echo pulse
  duration = pulseIn(echoPin, HIGH);

  // Calculate the distance (cm)
  distance = duration * 0.034 / 2;

  return distance;
}

// Function to handle sensor readings and notifications
void handleSensor(int sensorIndex) {
  if (distances[sensorIndex] > 0 && distances[sensorIndex] <= 10) {
    if (!ledStates[sensorIndex]) {
      if (timers[sensorIndex] == 0) {
        timers[sensorIndex] = millis(); // Start the timer
      } else if (millis() - timers[sensorIndex] >= 5000) {
        digitalWrite(ledPins[sensorIndex], HIGH); // Turn on physical LED
        Blynk.virtualWrite(virtualPins[sensorIndex], 255); // Turn on Blynk LED Widget

        // Send push notification
        String notificationMessage = (sensorIndex == 0) ? "Bin-BIO -- @GS is FULL" :
                                     (sensorIndex == 1) ? "Bin-NONBIO -- @GS is FULL" :
                                                          "Bin-Hazard -- @GS is FULL";
        Blynk.logEvent("sensor_alert", notificationMessage);

        ledStates[sensorIndex] = true; // Mark LED as active
      }
    }
  } else { // No object detected or out of range
    timers[sensorIndex] = 0; // Reset timer
    ledStates[sensorIndex] = false; // Reset LED state
    digitalWrite(ledPins[sensorIndex], LOW); // Turn off physical LED
    Blynk.virtualWrite(virtualPins[sensorIndex], 0); // Turn off Blynk LED Widget
  }
}








