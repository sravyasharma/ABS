#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();
unsigned long lastReceivedTime = 0;
int lastReceivedNumber = -1;

void setup() {
  Serial.begin(9600);
  Serial.println("RF Receiver - Waiting for Numbers (0-100)");

  mySwitch.enableReceive(D1);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  if (mySwitch.available()) {
    unsigned long receivedValue = mySwitch.getReceivedValue();

    if (receivedValue <= 100) { 
      Serial.print("Received Number: ");
      Serial.println(receivedValue);
      
      lastReceivedNumber = receivedValue;
      lastReceivedTime = millis();

      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
      digitalWrite(LED_BUILTIN, HIGH);
    }

    mySwitch.resetAvailable();
    delay(50);
  }

  // Ensure message loss detection doesn't trigger incorrectly when restarting from 0
  if (millis() - lastReceivedTime > 2000) {
    Serial.println("Message NOT received!");
    lastReceivedTime = millis(); 
  }

  delay(10);
}