//This code is made to test the RF Transreciever where the Transmitter sends a number in the range of 0-100 for every second and reciever takes it.
//Problems Found: Range is 700m and numbers such as 0 are not recieved. Line of sight must be clear.
#include <RCSwitch.h>
RCSwitch mySwitch = RCSwitch();
int numberToSend = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("RF Transmitter - Sending Numbers (0-100)");

  mySwitch.enableTransmit(D2);
  mySwitch.setProtocol(1);
  mySwitch.setPulseLength(200);
  mySwitch.setRepeatTransmit(5);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  Serial.print("Sending number: ");
  Serial.println(numberToSend);

  digitalWrite(LED_BUILTIN, LOW);
  mySwitch.send(numberToSend, 24);
  digitalWrite(LED_BUILTIN, HIGH);

  numberToSend++;
  if (numberToSend > 100) {
    numberToSend = 0; // Restart cycle after 100
  }

  delay(1000); // Send message every 1 second
}