// Receiver ESP32 Code for Borewell Water Level Monitoring and Motor Control
// This code runs on the ESP32 connected to a LoRa SX1278 module, a relay, and control buttons.
// It receives water level data wirelessly from the Transmitter ESP32,
// controls the motor based on water level (auto mode) or manual input (manual mode),
// and integrates with Blynk for remote control and monitoring.

// --- Libraries ---
#include <SPI.h>           // Required for LoRa communication
#include <LoRa.h>          // LoRa library for SX1278 modules
#include <WiFi.h>          // WiFi library for ESP32
#include <BlynkSimpleEsp32.h> // Blynk library for ESP32

// --- WiFi and Blynk Credentials ---
char ssid[] = "Vaasudeva Sarvam"; // Your WiFi SSID
char pass[] = "1234567890";     // Your WiFi password
char auth[] = "j1ynBsmi6uq8AfO2N2T-WLsupoBgovWc"; // Your Blynk Auth Token

// --- LoRa Configuration ---
// Define LoRa pins for ESP32. These are common pin assignments, but may vary based on your specific ESP32 board.
// Ensure these match the Transmitter's LoRa pin definitions.
#define LORA_SCK    5   // LoRa SCK pin connected to ESP32 GPIO5
#define LORA_MISO   19  // LoRa MISO pin connected to ESP32 GPIO19
#define LORA_MOSI   27  // LoRa MOSI pin connected to ESP32 GPIO27
#define LORA_CS     18  // LoRa Chip Select (NSS) pin connected to ESP32 GPIO18
#define LORA_RST    14  // LoRa Reset pin connected to ESP32 GPIO14
#define LORA_DIO0   26  // LoRa DIO0 (Interrupt) pin connected to ESP32 GPIO26

const long LORA_FREQUENCY = 433E6; // LoRa frequency (must match Transmitter)

// --- Pin Definitions ---
#define RELAY_PIN         25  // ESP32 GPIO pin connected to the Relay module's IN / PORTS pin
#define MANUAL_BUTTON_PIN 34  // ESP32 GPIO pin connected to the Manual control button (e.g., for physical override)
#define MODE_BUTTON_PIN   35  // ESP32 GPIO pin connected to the Mode switch button (Auto/Manual)

// --- Water Level Thresholds ---
#define WATER_LEVEL_HIGH_THRESHOLD 90 // Motor turns OFF if water level is >= 90%
#define WATER_LEVEL_LOW_THRESHOLD  30 // Motor turns ON if water level is <= 30%

// --- Timing Constants ---
const uint32_t DEBOUNCE_DELAY = 50; // Debounce delay for physical buttons

// --- State Variables ---
bool relayState = false;           // Current state of the relay (true = ON, false = OFF)
bool autoMode = false;             // Current operating mode (true = Auto, false = Manual)
int receivedGaugeValue = 0;        // Stores the water level percentage received from LoRa
uint32_t lastDebounceTime = 0;     // For button debouncing
int lastModeButtonState = HIGH;    // Previous state of the mode button
int modeButtonState;               // Current state of the mode button
int lastManualButtonState = HIGH;  // Previous state of the manual button

// --- Blynk Virtual Pins ---
#define VPIN_RELAY_CONTROL V1    // Virtual pin for Blynk button to control relay manually
#define VPIN_MODE_CONTROL  V2    // Virtual pin for Blynk button to switch modes (Auto/Manual)
#define VPIN_RELAY_STATUS  V3    // Virtual pin to display current relay status on Blynk
#define VPIN_MODE_STATUS   V4    // Virtual pin to display current mode status on Blynk
#define VPIN_DISTANCE      V5    // Virtual pin to display water level percentage on Blynk gauge/slider

// --- Function Prototypes ---
void onReceive(int packetSize); // LoRa packet reception callback
void toggleRelay();             // Toggles the relay state
void toggleMode();              // Toggles the operating mode (Auto/Manual)
void updateBlynkStatus();       // Updates Blynk widgets with current status
void handleAutoMode();          // Logic for automatic motor control based on water level
void handleManualMode();        // Logic for manual motor control
void checkModeButton();         // Checks the state of the physical mode button
void checkManualButton();       // Checks the state of the physical manual button

void setup() {
  Serial.begin(115200); // Initialize serial communication for debugging

  // Configure pin modes
  pinMode(RELAY_PIN, OUTPUT);
  // --- TEMPORARILY COMMENTED OUT FOR TROUBLESHOOTING GPIO ERROR ---
  // If the "GPIO number error" disappears after commenting these out,
  // there might be a wiring issue with your buttons or pin definitions.
  // Ensure you are connecting to the correct physical GPIO pins 34 and 35
  // on your specific ESP32 board.
  // pinMode(MANUAL_BUTTON_PIN, INPUT_PULLUP); // Use INPUT_PULLUP for buttons connected to GND
  // pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);   // Use INPUT_PULLUP for buttons connected to GND
  // --- END TEMPORARY COMMENT OUT ---


  // Set initial relay state to OFF (LOW for active-HIGH relays, as per observed behavior)
  digitalWrite(RELAY_PIN, LOW); // Set relay to OFF initially
  relayState = false;           // Ensure internal state matches physical state (false = OFF)

  // Initialize LoRa module
  Serial.println("Initializing LoRa...");
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS); // Initialize SPI bus for LoRa
  LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);        // Set LoRa module pins

  if (!LoRa.begin(LORA_FREQUENCY)) { // Start LoRa with the specified frequency
    Serial.println("Starting LoRa failed!");
    while (1); // Halt if LoRa initialization fails
  }
  Serial.println("LoRa initialized successfully!");
  LoRa.setSyncWord(0xF4); // Set a unique sync word (must match Transmitter)
  LoRa.onReceive(onReceive); // Register the callback function for received packets
  LoRa.receive();            // Put the LoRa module into receive mode initially

  // Connect to Blynk
  Serial.println("Connecting to Blynk...");
  Blynk.begin(auth, ssid, pass);
  Serial.println("Blynk connected!");


  Serial.println("System initialized.");
  // updateBlynkStatus() will be called by BLYNK_CONNECTED() for initial sync
}

// This function runs every time the ESP32 connects to the Blynk cloud
BLYNK_CONNECTED() {
  Serial.println("Blynk reconnected! Syncing all virtual pins.");
  // Request the latest state from the hardware to update all widgets in the app
  Blynk.syncAll(); // This will call BLYNK_WRITE for all pins that have a value stored on the server
                   // and also trigger BLYNK_READ for widgets configured for it (though we use PUSH)

  // Also manually update the status LEDs to ensure they reflect current state
  updateBlynkStatus();
}


void loop() {
  Blynk.run();         // Keep Blynk connection alive and process events

  // --- TEMPORARILY COMMENTED OUT FOR TROUBLESHOOTING GPIO ERROR ---
  // checkModeButton();   // Check the physical mode button
  // checkManualButton(); // Check the physical manual button
  // --- END TEMPORARY COMMENT OUT ---


  // Handle motor control based on the current mode
  if (autoMode) {
    handleAutoMode();
  } else {
    // Manual mode is handled by Blynk_WRITE and checkManualButton (if enabled)
  }

  // Periodically send the received gauge value to Blynk, ONLY if connected
  if (Blynk.connected()) {
    Blynk.virtualWrite(VPIN_DISTANCE, receivedGaugeValue);
  }
  // No need for Serial.print("Sending water level to Blynk V5: "); here, as LoRa Rx already prints the value.

  delay(10); // Small delay to prevent watchdog timer issues in a very fast loop
}

// --- LoRa Packet Reception Callback ---
void onReceive(int packetSize) {
  if (packetSize == 0) {
    Serial.println("LoRa Rx: No packet received.");
    LoRa.receive(); // Put LoRa back into receive mode
    return;
  }

  // Read packet
  String incomingData = "";
  while (LoRa.available()) {
    incomingData += (char)LoRa.read();
  }

  // Attempt to parse the received string as an integer (gauge value)
  int parsedValue = incomingData.toInt();

  // Validate the parsed value to prevent erroneous readings
  if (parsedValue >= 0 && parsedValue <= 100) {
    receivedGaugeValue = parsedValue;
    Serial.print("LoRa Rx: "); // Added explicit debugging
    Serial.print("Received Water Level: ");
    Serial.print(receivedGaugeValue);
    Serial.print("% | RSSI: ");
    Serial.println(LoRa.packetRssi()); // Added RSSI for debugging signal strength
  } else {
    Serial.print("LoRa Rx: Received invalid data: "); // Debugging for invalid packets
    Serial.println(incomingData);
  }

  LoRa.receive(); // !!! IMPORTANT FIX !!! Put LoRa back into receive mode after processing packet
}

// --- Relay Control Functions ---
void toggleRelay() {
  relayState = !relayState; // Invert the current relay state

  // !!! FIX for Active-HIGH Relay !!!
  // If relayState is true (motor ON), output HIGH. If false (motor OFF), output LOW.
  digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);

  Serial.print("Relay set to: ");
  Serial.println(relayState ? "ON" : "OFF");
  updateBlynkStatus(); // Update Blynk
}

// --- Mode Control Functions ---
void toggleMode() {
  autoMode = !autoMode; // Invert the current mode
  Serial.print("Switched to ");
  Serial.println(autoMode ? "Auto Mode" : "Manual Mode");
  updateBlynkStatus(); // Update Blynk
}

// --- Button Handling Functions ---
// These are temporarily commented out in setup() and loop() for troubleshooting
void checkModeButton() {
  int reading = digitalRead(MODE_BUTTON_PIN);

  // If the button state has changed, record the time for debouncing
  if (reading != lastModeButtonState) {
    lastDebounceTime = millis();
  }

  // If the debounce delay has passed and the button state is stable
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (reading != modeButtonState) {
      modeButtonState = reading; // Update current button state
      if (modeButtonState == LOW) { // Button is pressed (active low with INPUT_PULLUP)
        toggleMode(); // Toggle the operating mode
      }
    }
  }
  lastModeButtonState = reading; // Save the current reading for the next iteration
}

void checkManualButton() {
  // Only process manual button if in Manual Mode
  if (!autoMode) {
    int buttonState = digitalRead(MANUAL_BUTTON_PIN);

    // If button state changed, debounce
    if (buttonState != lastManualButtonState) {
      delay(DEBOUNCE_DELAY); // Simple delay for debouncing
      if (digitalRead(MANUAL_BUTTON_PIN) == buttonState) { // Confirm state after debounce
        if (buttonState == LOW) { // Button pressed
          toggleRelay(); // Toggle relay
        }
      }
    }
    lastManualButtonState = buttonState; // Save current state
  }
}

// --- Mode Logic Functions ---
void handleAutoMode() {
  // Turn ON if water level is below LOW_THRESHOLD and motor is currently OFF
  if (receivedGaugeValue <= WATER_LEVEL_LOW_THRESHOLD && !relayState) {
    relayState = true;
    digitalWrite(RELAY_PIN, HIGH); // Turn ON motor (Active-HIGH relay)
    Serial.println("Auto Mode: Water level < 30%, turning ON motor.");
    updateBlynkStatus();
    // Synchronize the manual control button (V1) in Blynk to reflect the auto-mode change
    if (Blynk.connected()) {
      Serial.println("Auto Mode: Syncing Blynk V1 to ON."); // Debug print
      Blynk.virtualWrite(VPIN_RELAY_CONTROL, 1); // Set V1 button to ON
    }
  }
  // Turn OFF if water level is above HIGH_THRESHOLD and motor is currently ON
  else if (receivedGaugeValue >= WATER_LEVEL_HIGH_THRESHOLD && relayState) {
    relayState = false;
    digitalWrite(RELAY_PIN, LOW); // Turn OFF motor (Active-HIGH relay)
    Serial.println("Auto Mode: Water level > 90%, turning OFF motor.");
    updateBlynkStatus();
    // Synchronize the manual control button (V1) in Blynk to reflect the auto-mode change
    if (Blynk.connected()) {
      Serial.println("Auto Mode: Syncing Blynk V1 to OFF."); // Debug print
      Blynk.virtualWrite(VPIN_RELAY_CONTROL, 0); // Set V1 button to OFF
    }
  }
}

// --- Blynk Functions ---
// This function is called when a command is sent from Blynk to VPIN_RELAY_CONTROL
BLYNK_WRITE(VPIN_RELAY_CONTROL) {
  int paramValue = param.asInt(); // Get the value from Blynk (0 or 1)
  Serial.print("BLYNK_WRITE V1 received: ");
  Serial.print(paramValue);
  Serial.print(", autoMode: ");
  Serial.print(autoMode ? "TRUE" : "FALSE");
  Serial.print(", current relayState: ");
  Serial.println(relayState ? "ON" : "OFF");

  if (!autoMode) { // If currently in MANUAL mode
    Serial.println("  -> In Manual Mode. Processing V1 command.");
    // Allow manual control
    if (paramValue == 1 && !relayState) { // If Blynk wants ON and motor is OFF
      toggleRelay();
    } else if (paramValue == 0 && relayState) { // If Blynk wants OFF and motor is ON
      toggleRelay();
    }
  } else { // If currently in AUTO mode
    Serial.println("  -> In Auto Mode. Blocking V1 command and resetting button state.");
    // Manual control is blocked in Auto Mode. Reset the Blynk button to reflect the actual hardware state.
    Blynk.virtualWrite(VPIN_RELAY_CONTROL, relayState ? 1 : 0); // Reset V1 button to current relay state
    Serial.println("Manual relay control blocked in Auto Mode. V1 button reset.");
  }
}

// This function is called when a command is sent from Blynk to VPIN_MODE_CONTROL
BLYNK_WRITE(VPIN_MODE_CONTROL) {
  int paramValue = param.asInt(); // Get the value from Blynk (0 for Manual, 1 for Auto)
  Serial.print("BLYNK_WRITE V2 received: ");
  Serial.print(paramValue);
  Serial.print(", current autoMode: ");
  Serial.println(autoMode ? "TRUE" : "FALSE");

  if (paramValue == 1 && !autoMode) { // If Blynk wants Auto (1) and not currently in Auto mode
    toggleMode();
  } else if (paramValue == 0 && autoMode) { // If Blynk wants Manual (0) and currently in Auto mode
    toggleMode();
  }
}

// This function updates the Blynk app with the current status of the relay and mode
void updateBlynkStatus() {
  if (Blynk.connected()) { // Only update if Blynk is connected
    Blynk.virtualWrite(VPIN_RELAY_STATUS, relayState ? 1 : 0); // Send 1 for ON, 0 for OFF to V3
    Blynk.virtualWrite(VPIN_MODE_STATUS, autoMode ? 1 : 0);    // Send 1 for Auto, 0 for Manual to V4
    Serial.print("Blynk Status Updated: Relay V3=");
    Serial.print(relayState ? "ON" : "OFF");
    Serial.print(", Mode V4=");
    Serial.println(autoMode ? "Auto" : "Manual");
  }
}
