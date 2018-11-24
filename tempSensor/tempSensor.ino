//#include <SPI.h>
//#include <nRF24L01.h>
#include <RF24.h>

/*
 * For ATTinty84, use board from Spence Konde, with Counterclockwise pins
 * Use Android as ISP (did not have to pick Tiny version)
 * 8 MHz.
 * 
 * Looks like the pinout is counter clockwise:
 * Vcc   Gnd
 * 0      10
 * 1       9
 * -       8
 * 2 .     7
 * 3       6
 * 4 .     5
 */

// TEMPERATURE SENSOR
#include <OneWire.h>
#include <DallasTemperature.h>

// ARDUINO is 2
// ATTINY84 is 9
#define ONE_WIRE_BUS 9

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// SENSOR CONFIG
#define SENSOR_ID 102
#define SENSOR_TYPE 1

// #define DEBUG

// RADIO
// ARDUINO is 9 and 8
// ATTINY84 is 8 and 7
#define CE_PIN  8
#define CSN_PIN 7

RF24 radio(CE_PIN,CSN_PIN);

byte gateway[5] = "ABCDE";

void setup() {
#ifdef DEBUG
  Serial.begin(115200);
  Serial.println("Starting");
#endif

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
#ifdef DEBUG
  Serial.print("Sent... ");
  if (result) {
    Serial.println("fail");
  } else {
    Serial.println("success");
  }
#endif
}

float getTemperature() {
  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(0);
#ifdef DEBUG
  Serial.print(temp);
#endif
  return temp;
}
