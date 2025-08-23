// Transmitter ESP32 Code for Borewell Water Level Monitoring
// This code runs on the ESP32 connected to the HC-SR04 ultrasonic sensor and a LoRa SX1278 module.
// It reads the water level, calculates a gauge percentage, and sends it wirelessly via LoRa to the Receiver ESP32.

// --- Libraries ---
#include <SPI.h>       // Required for LoRa communication
#include <LoRa.h>      // LoRa library for SX1278 modules
#include <NewPing.h>   // Library for HC-SR04 ultrasonic sensor

// --- LoRa Configuration ---
// Define LoRa pins for ESP32. These are common pin assignments, but may vary based on your specific ESP32 board.
// Check your ESP32 board's pinout for SPI (SCK, MISO, MOSI) and ensure you connect LoRa NSS, RST, and DIO0 to available GPIOs.
#define LORA_SCK    5   // LoRa SCK pin connected to ESP32 GPIO5
#define LORA_MISO   19  // LoRa MISO pin connected to ESP32 GPIO19
#define LORA_MOSI   27  // LoRa MOSI pin connected to ESP32 GPIO27
#define LORA_CS     18  // LoRa Chip Select (NSS) pin connected to ESP32 GPIO18
#define LORA_RST    14  // LoRa Reset pin connected to ESP32 GPIO14
#define LORA_DIO0   26  // LoRa DIO0 (Interrupt) pin connected to ESP32 GPIO26

const long LORA_FREQUENCY = 433E6; // LoRa frequency (e.g., 433E6 for 433MHz, 868E6 for 868MHz, 915E6 for 915MHz)
                                   // Ensure both Transmitter and Receiver use the same frequency.

// --- Ultrasonic Sensor Pin Definitions (HC-SR04) ---
#define TRIG_PIN    32  // ESP32 GPIO pin connected to HC-SR04 TRIG
#define ECHO_PIN    33  // ESP32 GPIO pin connected to HC-SR04 ECHO
// !!! IMPORTANT FOR HC-SR04 !!!
// The HC-SR04 ECHO pin outputs a 5V signal, but ESP32 GPIOs are 3.3V tolerant.
// Connecting 5V directly to a 3.3V tolerant pin can damage the ESP32 over time or cause erratic readings.
// It is STRONGLY RECOMMENDED to use a voltage divider on the ECHO pin to safely reduce the 5V signal to ~3.3V.
// Example for Voltage Divider (HC-SR04 ECHO -> ESP32 GPIO33):
// HC-SR04 ECHO Pin --[1k Ohm Resistor]--+-- ESP32 GPIO33
//                                      |
//                                      ---[2k Ohm Resistor]--- GND
// This setup will reduce the 5V signal to approximately 3.33V (5V * (2k / (1k + 2k))), which is safe for ESP32.

// --- Ultrasonic Sensor Constants ---
#define MAX_DISTANCE 200 // Maximum distance (in cm) the sensor will measure. Adjust based on your borewell depth.
#define MIN_DISTANCE 2   // Minimum distance (in cm) the sensor can accurately measure.
#define CLOSE_RANGE 50   // Distance (in cm) below which the water level is considered "close" to the sensor.
                         // This is used for the enhanced gauge calculation.

// Initialize ultrasonic sensor object
NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);

// --- Timing Constants ---
const uint32_t SENSOR_READ_INTERVAL = 5000; // Interval (in milliseconds) to read sensor and send data (e.g., 5000ms = 5 seconds)

// --- State Variables ---
uint32_t lastSensorReadMillis = 0; // Stores the last time the sensor was read and data sent
int lastGaugeValue = 0;            // Stores the last calculated water level percentage

// --- Function to calculate enhanced gauge value ---
// This function takes the raw distance reading from the ultrasonic sensor
// and converts it into a percentage representing the water level.
// It uses a non-linear mapping for the "close range" to provide more detail
// when the water is near the sensor (high level).
int calculateEnhancedGauge(unsigned int distance) {
  // If distance is 0 (timeout/error) or beyond max distance, return 0% (empty)
  if (distance == 0 || distance > MAX_DISTANCE) {
    Serial.print("Sensor reading out of range or timeout (");
    Serial.print(distance);
    Serial.println(" cm). Returning 0% water level.");
    return 0; // Indicates no valid reading or completely empty
  }

  // Constrain distance within defined min/max range
  distance = constrain(distance, MIN_DISTANCE, MAX_DISTANCE);

  int gaugeValue;
  if (distance <= CLOSE_RANGE) {
    // If water is in the "close range" (high level), use a non-linear scale
    // to give more resolution for the top 50% of the gauge.
    float percentage = (float)(CLOSE_RANGE - distance) / (CLOSE_RANGE - MIN_DISTANCE);
    gaugeValue = 50 + (percentage * percentage * 50); // Quadratic curve for upper half
  } else {
    // If water is below the "close range" (lower level), use linear mapping
    // for the bottom 50% of the gauge.
    gaugeValue = map(distance, MAX_DISTANCE, CLOSE_RANGE, 0, 50);
  }

  // Ensure the gauge value is always between 0 and 100
  return constrain(gaugeValue, 0, 100);
}

void setup() {
  Serial.begin(115200); // Initialize serial communication for debugging

  // Initialize LoRa module
  Serial.println("Initializing LoRa...");
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS); // Initialize SPI bus for LoRa
  LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);        // Set LoRa module pins

  if (!LoRa.begin(LORA_FREQUENCY)) { // Start LoRa with the specified frequency
    Serial.println("Starting LoRa failed!");
    while (1); // Halt if LoRa initialization fails
  }
  Serial.println("LoRa initialized successfully!");
  LoRa.setSyncWord(0xF4); // Set a unique sync word to prevent interference from other LoRa devices
                          // Both transmitter and receiver must use the same sync word.
  Serial.println("LoRa sync word set to 0xF4");
}

void loop() {
  uint32_t currentMillis = millis(); // Get current time

  // Check if it's time to read the sensor and send data
  if (currentMillis - lastSensorReadMillis >= SENSOR_READ_INTERVAL) {
    lastSensorReadMillis = currentMillis; // Update last read time

    // Perform ultrasonic distance measurement
    unsigned int distance = sonar.ping_cm(); // Get distance in centimeters

    // Calculate the water level gauge value
    lastGaugeValue = calculateEnhancedGauge(distance);

    // Print raw distance and calculated gauge value to Serial Monitor
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.print(" cm | Gauge Value: ");
    Serial.print(lastGaugeValue);
    Serial.println("%");

    // Send the gauge value via LoRa
    Serial.print("Sending water level: ");
    Serial.print(lastGaugeValue);
    Serial.println("%");

    LoRa.beginPacket();             // Start LoRa packet transmission
    LoRa.print(lastGaugeValue);     // Write the gauge value to the packet
    LoRa.endPacket();               // Finish LoRa packet transmission

    Serial.println("Data sent!");
  }
  delay(10); // Small delay to prevent watchdog timer issues in a very fast loop
}
