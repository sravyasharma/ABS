#include <BlynkSimpleEsp8266.h>
#include <AceButton.h>

#define BLYNK_TEMPLATE_ID "TMPL3j4rDkfRA"
#define BLYNK_TEMPLATE_NAME "BMS"
#define BLYNK_AUTH_TOKEN "wUQ6cYqKvjE4ACSm6Q6aynPRSIjYtsyg"

char ssid[] = "Sarvavyapi";
char pass[] = "Sairam#009";

using namespace ace_button;

#define Relay 14  // D5
#define BP1 2     // D0
#define BP2 13    // D7
#define BP3 15    // D8

#define V_B_3 V3
#define V_B_4 V4

bool toggleRelay = false;
bool modeFlag = true;
String currMode;

char auth[] = BLYNK_AUTH_TOKEN;

ButtonConfig config1;
AceButton button1(&config1);
ButtonConfig config2;
AceButton button2(&config2);
ButtonConfig config3;
AceButton button3(&config3);

void handleEvent1(AceButton*, uint8_t, uint8_t);
void handleEvent2(AceButton*, uint8_t, uint8_t);
void handleEvent3(AceButton*, uint8_t, uint8_t);

BlynkTimer timer;

void checkBlynkStatus() {
  bool isconnected = Blynk.connected();
  // Add any Blynk connection status handling here if needed
}

BLYNK_WRITE(VPIN_BUTTON_3) {
  modeFlag = param.asInt();
  if (!modeFlag && toggleRelay) {
    digitalWrite(Relay, LOW);
    toggleRelay = false;
  }
  currMode = modeFlag ? "AUTO" : "MANUAL";
}

BLYNK_WRITE(VPIN_BUTTON_4) {
  if (!modeFlag) {
    toggleRelay = param.asInt();
    digitalWrite(Relay, toggleRelay);
  } else {
    Blynk.virtualWrite(V_B_4, toggleRelay);
  }
}

BLYNK_CONNECTED() {
  Blynk.syncVirtual(V_B_1);
  Blynk.virtualWrite(V_B_3, modeFlag);
  Blynk.virtualWrite(V_B_4, toggleRelay);
}

void setup() {
  Serial.begin(9600);
  pinMode(Relay, OUTPUT);

  pinMode(BP1, INPUT_PULLUP);
  pinMode(BP2, INPUT_PULLUP);
  pinMode(BP3, INPUT_PULLUP);

  digitalWrite(Relay, HIGH);

  config1.setEventHandler(button1Handler);
  config2.setEventHandler(button2Handler);
  config3.setEventHandler(button3Handler);

  button1.init(BP1);
  button2.init(BP2);
  button3.init(BP3);

  currMode = modeFlag ? "AUTO" : "MANUAL";

  WiFi.begin(ssid, pass);
  timer.setInterval(2000L, checkBlynkStatus);

  Blynk.config(auth);
  delay(1000);

  Blynk.virtualWrite(V_B_3, modeFlag);
  Blynk.virtualWrite(V_B_4, toggleRelay);

  delay(500);
}

void loop() {
  Blynk.run();
  timer.run();
  button1.check();
  button3.check();

  if (!modeFlag) {
    button2.check();
  }
}

void button1Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.println("EVENT1");
  switch (eventType) {
    case AceButton::kEventReleased:
      if (modeFlag && toggleRelay) {
        digitalWrite(Relay, LOW);
        toggleRelay = false;
      }
      modeFlag = !modeFlag;
      currMode = modeFlag ? "AUTO" : "MANUAL";
      Blynk.virtualWrite(V_B_3, modeFlag);
      break;
  }
}

void button2Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.println("EVENT2");
  switch (eventType) {
    case AceButton::kEventReleased:
      if (toggleRelay) {
        digitalWrite(Relay, LOW);
        toggleRelay = false;
      } else {
        digitalWrite(Relay, HIGH);
        toggleRelay = true;
      }
      Blynk.virtualWrite(V_B_4, toggleRelay);
      delay(1000);
      break;
  }
}

void button3Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.println("EVENT3");
  // Add functionality for button 3 if required.
}