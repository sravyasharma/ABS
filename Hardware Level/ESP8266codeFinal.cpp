#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// Your WiFi credentials
char ssid[] = "Sarvavyapi";
char pass[] = "Sairam#009";

// You should get Auth Token in the Blynk App.
char auth[] = "cLjui-Mq7V4_EzqTGT-YyVV2QKanmajF";

// Pin Definitions
#define RELAY_PIN D5        // Relay pin (GPIO14)
#define MANUAL_BUTTON_PIN D6 // Manual control button (GPIO12)
#define MODE_BUTTON_PIN D0   // Mode selection button (GPIO16)

// Constants
const uint32_t INTERVAL = 5000; // Changed to 5 seconds
const uint32_t DEBOUNCE_DELAY = 50; // 50 ms debounce time

// Variables
bool relayState = false;
bool autoMode = false;
uint32_t previousMillis = 0;
uint32_t lastDebounceTime = 0;
int lastModeButtonState = HIGH;
int modeButtonState;

// Blynk virtual pins
#define VPIN_RELAY_CONTROL V1
#define VPIN_MODE_CONTROL V2
#define VPIN_RELAY_STATUS V3
#define VPIN_MODE_STATUS V4

void setup() {
  Serial.begin(115200); // Faster baud rate for debugging
  
  // Pin Setup
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(MANUAL_BUTTON_PIN, INPUT_PULLUP);
  pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);
  
  // Ensure relay is off initially (active-low)
  digitalWrite(RELAY_PIN, HIGH);
  
  // Connect to WiFi and Blynk
  Blynk.begin(auth, ssid, pass);
  
  Serial.println("System initialized in Manual Mode");
  updateBlynkStatus();
}

void loop() {
  Blynk.run();
  checkModeButton();
  
  if (autoMode) {
    handleAutoMode();
  } else {
    handleManualMode();
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
    previousMillis = millis(); // Reset the timer when entering auto mode
    Serial.println("Switched to Auto Mode");
  } else {
    Serial.println("Switched to Manual Mode");
  }
  updateBlynkStatus();
}

void handleAutoMode() {
  uint32_t currentMillis = millis();
  if (currentMillis - previousMillis >= INTERVAL) {
    previousMillis = currentMillis;
    toggleRelay();
  }
}

void handleManualMode() {
  if (digitalRead(MANUAL_BUTTON_PIN) == LOW) {
    delay(DEBOUNCE_DELAY); // Simple debounce
    if (digitalRead(MANUAL_BUTTON_PIN) == LOW) {
      toggleRelay();
      while (digitalRead(MANUAL_BUTTON_PIN) == LOW) {
        // Wait for button release
      }
    }
  }
}

void toggleRelay() {
  relayState = !relayState;
  digitalWrite(RELAY_PIN, relayState ? LOW : HIGH); // Active-low relay control
  Serial.println(relayState ? "Relay ON" : "Relay OFF");
  updateBlynkStatus();
}

void updateBlynkStatus() {
  Blynk.virtualWrite(VPIN_RELAY_STATUS, relayState);
  Blynk.virtualWrite(VPIN_MODE_STATUS, autoMode);
}

// Blynk functions to handle app inputs
BLYNK_WRITE(VPIN_RELAY_CONTROL) {
  if (!autoMode) {
    relayState = param.asInt();
    digitalWrite(RELAY_PIN, relayState ? LOW : HIGH);
    updateBlynkStatus();
  }
}

BLYNK_WRITE(VPIN_MODE_CONTROL) {
  autoMode = param.asInt();
  if (autoMode) {
    previousMillis = millis(); // Reset the timer when entering auto mode
  }
  updateBlynkStatus();
}