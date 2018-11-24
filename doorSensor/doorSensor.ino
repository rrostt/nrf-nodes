// #include <SPI.h>
// #include <nRF24L01.h>
#include <RF24.h>

// SENSOR CONFIG
#define SENSOR_ID 102
#define SENSOR_TYPE 2

#define DEBUG

// DOOR PIN
#define DOOR_PIN 3

// RADIO
#define CE_PIN  9
#define CSN_PIN 8

RF24 radio(CE_PIN, CSN_PIN);

byte gateway[5] = "ABCDE";

void setup() {
#ifdef DEBUG
  Serial.begin(115200);
  Serial.println("Starting");
#endif

  pinMode(DOOR_PIN, INPUT_PULLUP);

  radio.begin();
  radio.setDataRate( RF24_250KBPS );
  radio.setPALevel(RF24_PA_HIGH);

  radio.openWritingPipe(gateway);
}

uint8_t message[32];
int prev = false;

void loop() {
  int current = digitalRead(DOOR_PIN);

  prepareMessage(current);
  sendMessage();

  delay(1000);
}

void prepareMessage(int state) {
  message[0] = SENSOR_ID;
  message[1] = SENSOR_TYPE; // temperature
  message[2] = 255; // battery % or 255 for unavailable
  message[3] = state == HIGH ? 1 : 0;
}

void sendMessage() {
  bool result = radio.write(&message, sizeof(message));
#ifdef DEBUG
  Serial.print("Sent... ");
  if (result) {
    Serial.println("fail");
  } else {
    Serial.println("success");
  }
#endif
}
