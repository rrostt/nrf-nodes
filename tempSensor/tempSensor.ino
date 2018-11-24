#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// TEMPERATURE SENSOR
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 2

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// SENSOR CONFIG
#define SENSOR_ID 100
#define SENSOR_TYPE 1

// RADIO
#define CE_PIN   9
#define CSN_PIN 8

RF24 radio(CE_PIN,CSN_PIN);

byte gateway[5] = "ABCDE";

void setup() {
  Serial.begin(115200);
  Serial.println("Starting");

  sensors.begin();

  radio.begin();
  radio.setDataRate( RF24_250KBPS );
  radio.setPALevel(RF24_PA_HIGH);

  radio.openWritingPipe(gateway);
}

uint8_t message[32];

void loop() {
  prepareMessage(getTemperature()*100);
  sendMessage();
  delay(200);
}

void prepareMessage(float temp) {
  message[0] = SENSOR_ID;
  message[1] = SENSOR_TYPE; // temperature
  message[2] = 255; // battery % or 255 for unavailable
  message[3] = ((uint16_t)temp) >> 8;
  message[4] = ((uint16_t)temp) & 0xff;
}

void sendMessage() {
  bool result = radio.write(&message, sizeof(message));
  Serial.print("Sent... ");
  if (result) {
    Serial.println("fail");
  } else {
    Serial.println("success");
  }
}

float getTemperature() {
  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(0);
  Serial.print(temp);
  return temp;
}
