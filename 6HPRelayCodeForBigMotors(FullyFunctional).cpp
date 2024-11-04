#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <NewPing.h>

// WiFi and Blynk credentials
char ssid[] = "Sarvavyapi";
char pass[] = "Sairam#009";
char auth[] = "cLjui-Mq7V4_EzqTGT-YyVV2QKanmajF";

// Pin Definitions
#define TRIG_PIN D1
#define ECHO_PIN D2
#define RELAY_PIN D5
#define MANUAL_BUTTON_PIN D6
#define MODE_BUTTON_PIN D0

// Ultrasonic sensor constants
#define MAX_DISTANCE 200
#define MIN_DISTANCE 2
#define CLOSE_RANGE 50

// Initialize ultrasonic sensor
NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);

// Timing constants
const uint32_t DEBOUNCE_DELAY = 50;
const uint32_t SENSOR_INTERVAL = 1000;

// State variables
bool relayState = true;  // Changed initial state to true (OFF for 6HP relay)
bool autoMode = false;
uint32_t lastSensorMillis = 0;
uint32_t lastDebounceTime = 0;
int lastModeButtonState = HIGH;
int modeButtonState;
int lastGaugeValue = 0;

// Virtual Pins
#define VPIN_RELAY_CONTROL V1
#define VPIN_MODE_CONTROL V2
#define VPIN_RELAY_STATUS V3
#define VPIN_MODE_STATUS V4
#define VPIN_DISTANCE V5

// Function to calculate enhanced gauge value
int calculateEnhancedGauge(unsigned int distance) {
  if (distance == 0 || distance > MAX_DISTANCE) {
    return 0;
  }
  
  distance = constrain(distance, MIN_DISTANCE, MAX_DISTANCE);
  
  int gaugeValue;
  if (distance <= CLOSE_RANGE) {
    float percentage = (float)(CLOSE_RANGE - distance) / (CLOSE_RANGE - MIN_DISTANCE);
    gaugeValue = 50 + (percentage * percentage * 50);
  } else {
    gaugeValue = map(distance, MAX_DISTANCE, CLOSE_RANGE, 0, 50);
  }
  
  return constrain(gaugeValue, 0, 100);
}

void setup() {
  Serial.begin(115200);
  
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(MANUAL_BUTTON_PIN, INPUT_PULLUP);
  pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);
  
  // Initialize relay to OFF state (LOW for new logic)
  digitalWrite(RELAY_PIN, LOW);  // Changed to LOW
  relayState = true;  // true now means OFF
  
  Blynk.begin(auth, ssid, pass);
  
  Serial.println("System initialized in Manual Mode");
  updateBlynkStatus();
}

void loop() {
  Blynk.run();
  checkModeButton();
  
  updateUltrasonicReadings();
  
  if (autoMode) {
    handleAutoMode();
  } else {
    handleManualMode();
  }
}

void updateUltrasonicReadings() {
  uint32_t currentMillis = millis();
  if (currentMillis - lastSensorMillis >= SENSOR_INTERVAL) {
    lastSensorMillis = currentMillis;
    
    unsigned int distance = sonar.ping_cm();
    lastGaugeValue = calculateEnhancedGauge(distance);
    
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.print(" cm | Gauge Value: ");
    Serial.print(lastGaugeValue);
    Serial.println("%");
    
    Blynk.virtualWrite(VPIN_DISTANCE, lastGaugeValue);
  }
}

void checkModeButton() {
  int reading = digitalRead(MODE_BUTTON_PIN);
  
  if (reading != lastModeButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (reading != modeButtonState) {
      modeButtonState = reading;
      if (modeButtonState == LOW) {
        toggleMode();
      }
    }
  }
  
  lastModeButtonState = reading;
}

void toggleMode() {
  autoMode = !autoMode;
  
  if (autoMode) {
    Serial.println("Switched to Auto Mode - Operating on sensor readings");
  } else {
    Serial.println("Switched to Manual Mode");
  }
  
  Blynk.virtualWrite(VPIN_MODE_CONTROL, autoMode);
  updateBlynkStatus();
}

void handleAutoMode() {
  // Turn ON if gauge is less than 30% (low water level)
  if (lastGaugeValue < 30 && relayState) {  // Changed condition
    setRelayState(false);  // Changed to false for ON
    Serial.println("Auto Mode: Low water level detected (< 30%), turning ON");
  }
  // Turn OFF if gauge is greater than 85% (high water level)
  else if (lastGaugeValue > 85 && !relayState) {  // Changed condition
    setRelayState(true);  // Changed to true for OFF
    Serial.println("Auto Mode: High water level detected (> 85%), turning OFF");
  }
}

void handleManualMode() {
  static int lastButtonState = HIGH;
  int buttonState = digitalRead(MANUAL_BUTTON_PIN);
  
  if (buttonState != lastButtonState) {
    delay(DEBOUNCE_DELAY);
    if (digitalRead(MANUAL_BUTTON_PIN) == buttonState) {
      if (buttonState == LOW) {
        toggleRelay();
      }
    }
  }
  lastButtonState = buttonState;
}

void toggleRelay() {
  setRelayState(!relayState);
}

void setRelayState(bool state) {
  relayState = state;
  // New logic: 
  // relayState true = OFF = LOW signal
  // relayState false = ON = HIGH signal
  digitalWrite(RELAY_PIN, !relayState);
  
  // For Blynk, we invert the state so it shows correctly
  Blynk.virtualWrite(VPIN_RELAY_CONTROL, !relayState);
  Serial.println(relayState ? "Relay OFF" : "Relay ON");
  updateBlynkStatus();
}

void updateBlynkStatus() {
  // Invert relayState when sending to Blynk
  Blynk.virtualWrite(VPIN_RELAY_STATUS, !relayState);
  Blynk.virtualWrite(VPIN_MODE_STATUS, autoMode);
}

BLYNK_WRITE(VPIN_RELAY_CONTROL) {
  if (!autoMode) {
    // Invert the incoming state from Blynk
    setRelayState(!param.asInt());
  }
}

BLYNK_WRITE(VPIN_MODE_CONTROL) {
  autoMode = param.asInt();
  updateBlynkStatus();
}
