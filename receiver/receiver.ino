#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// SENSOR CONFIG
#define SENSOR_ID 101
#define SENSOR_TYPE 2

// RADIO
#define CE_PIN  9
#define CSN_PIN 8

RF24 radio(CE_PIN,CSN_PIN);

byte gateway[5] = "ABCDE";
byte ownAddress[5] = { 'E', 'D', 'C', 'B', SENSOR_ID };

void setup() {
  Serial.begin(115200);
  Serial.println("Starting");

  radio.begin();
  radio.setDataRate( RF24_250KBPS );
  radio.setPALevel(RF24_PA_HIGH);

  radio.openWritingPipe(gateway);
  radio.openReadingPipe(1, ownAddress);

  radio.startListening();

  pinMode(3, OUTPUT);
  digitalWrite(3, LOW);
}

uint8_t message[32];

void loop() {
  if (radio.available()) {
    radio.read(&message, sizeof(message));
    radio.flush_tx();
    digitalWrite(3, HIGH);
  }
}
