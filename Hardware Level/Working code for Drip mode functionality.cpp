#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <NewPing.h>

#define TRIG_PIN D1
#define ECHO_PIN D2
#define RELAY_PIN D5
#define MANUAL_BUTTON_PIN D6
#define MODE_BUTTON_PIN D0

#define MAX_DISTANCE 200
#define MIN_DISTANCE 2
#define CLOSE_RANGE 50

#define VPIN_RELAY_CONTROL V1
#define VPIN_MODE_CONTROL V2
#define VPIN_RELAY_STATUS V3
#define VPIN_MODE_STATUS V4
#define VPIN_DISTANCE V5
#define VPIN_DRIP_CONTROL V6  // New virtual pin for Drip button

char ssid[] = "Sarvavyapi";
char pass[] = "Sairam#009";
char auth[] = "cLjui-Mq7V4_EzqTGT-YyVV2QKanmajF";

NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);
const uint32_t DEBOUNCE_DELAY = 50;
const uint32_t SENSOR_INTERVAL = 1000;

bool relayState = true;
bool autoMode = false;
bool dripEnabled = false;  // Tracks Drip button state
uint32_t lastSensorMillis = 0;
uint32_t lastDebounceTime = 0;
int lastModeButtonState = HIGH;
int modeButtonState;
int lastGaugeValue = 0;

void setup() {
  Serial.begin(115200);
  
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(MANUAL_BUTTON_PIN, INPUT_PULLUP);
  pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);
  
  digitalWrite(RELAY_PIN, LOW);
  relayState = true;
  
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

void handleAutoMode() {
  static uint32_t dripTimer = 0;  // Timer for Drip functionality

  if (lastGaugeValue < 30 && relayState) {
    setRelayState(false);
    Serial.println("Auto Mode: Low water level detected, turning ON");
  } 
  else if (lastGaugeValue > 95 && !relayState) {
    if (dripEnabled) {
      if (dripTimer == 0) {
        dripTimer = millis();  // Start the 20-second timer
        Serial.println("Auto Mode: Drip mode enabled, extending motor ON for 20 seconds");
      } 
      else if (millis() - dripTimer >= 20000) {
        setRelayState(true);  // Turn off relay after 20 seconds
        dripTimer = 0;  // Reset timer
        dripEnabled = false;  // Turn off drip mode
        autoMode = false;  // Turn off auto mode after drip timer completes
        Blynk.virtualWrite(VPIN_DRIP_CONTROL, 0);  // Update Blynk to show drip mode off
        Blynk.virtualWrite(VPIN_MODE_CONTROL, 0);  // Update Blynk to show auto mode off
        Serial.println("Auto Mode: Drip timer expired, turning OFF relay, Drip mode and Auto mode");
      }
    } 
    else {
      setRelayState(true);  // Turn off immediately if Drip is disabled
      Serial.println("Auto Mode: High water level detected, turning OFF");
    }
  } 
  else {
    dripTimer = 0;  // Reset timer if water level is below 95%
  }
}

void handleManualMode() {
  static int lastButtonState = HIGH;
  int buttonState = digitalRead(MANUAL_BUTTON_PIN);
  
  if (buttonState != lastButtonState) {
    delay(DEBOUNCE_DELAY);
    if (digitalRead(MANUAL_BUTTON_PIN) == buttonState && buttonState == LOW) {
      toggleRelay();
    }
  }
  lastButtonState = buttonState;
}

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

void toggleRelay() {
  setRelayState(!relayState);
}

void setRelayState(bool state) {
  relayState = state;
  digitalWrite(RELAY_PIN, !relayState);
  Blynk.virtualWrite(VPIN_RELAY_CONTROL, !relayState);
  Serial.println(relayState ? "Relay OFF" : "Relay ON");
  updateBlynkStatus();
}

void updateBlynkStatus() {
  Blynk.virtualWrite(VPIN_RELAY_STATUS, !relayState);
  Blynk.virtualWrite(VPIN_MODE_STATUS, autoMode);
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
  Serial.println(autoMode ? "Switched to Auto Mode" : "Switched to Manual Mode");
  Blynk.virtualWrite(VPIN_MODE_CONTROL, autoMode);
  updateBlynkStatus();
}

BLYNK_WRITE(VPIN_RELAY_CONTROL) {
  if (!autoMode) {
    setRelayState(!param.asInt());
  }
}

BLYNK_WRITE(VPIN_MODE_CONTROL) {
  autoMode = param.asInt();
  updateBlynkStatus();
}

BLYNK_WRITE(VPIN_DRIP_CONTROL) {
  dripEnabled = param.asInt();  // Update Drip button state
  
  if (dripEnabled) {
    autoMode = true;  // Automatically turn on auto mode when Drip is enabled
    Blynk.virtualWrite(VPIN_MODE_CONTROL, 1);  // Update auto mode status on Blynk
    Serial.println("Drip Mode ON - Auto Mode also enabled");
  } else {
    Serial.println("Drip Mode OFF");
  }
  
  updateBlynkStatus();
}
