#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#include <printf.h>

// TEMPERATURE SENSOR
#include <OneWire.h>
#include <DallasTemperature.h>

// ARDUINO is 2
// ATTINY84 is 9
#ifdef ATTINY_CORE
#define ONE_WIRE_BUS 9
#else
#define ONE_WIRE_BUS 2
#endif

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// SENSOR CONFIG (for temp sensor)
#define SENSOR_ID 100
#define SENSOR_TYPE 1

// RADIO

#define CE_PIN  9
#define CSN_PIN 8

RF24 radio(CE_PIN,CSN_PIN);

byte address[5] = "ABCDE";
byte sendToAddress[5] = "EDCBA";

bool hasTemp;

void setup() {
  Serial.begin(115200);
  printf_begin();
  Serial.println("Start receiving");

  sensors.begin();
  hasTemp = sensors.getDS18Count() > 0;

  radio.begin();
  radio.setDataRate( RF24_250KBPS );

  radio.openReadingPipe(1, address);
  radio.printDetails();
  
  radio.startListening();
}

uint8_t message[32];
char output[200];
char incoming[37];
int incomingPosition = 0;
unsigned long lastMillis = 0;

void loop() {
  if (radio.available()) {
    radio.read(&message, sizeof(message));
    radio.flush_tx();
    radio.stopListening();
    outputMessage();
    radio.startListening();
  }
  if (hasTemp) {
    handleTemperature();
  }
  if (Serial.available() > 0) {
    handleIncoming();
  }
}
  
void formOutput() {
  int i = 0;
  output[0] = 0;
  for (i = 0; i < 31; i++) {
    sprintf(output + strlen(output), "%d, ", message[i]);
  }
  sprintf(output + strlen(output), "%d", message[31]);
}

void outputMessage() {
  Serial.print("Data ");
  formOutput();
  Serial.println(output);
}

void handleIncoming() {
  incoming[incomingPosition++] = Serial.read();
  if (incomingPosition >= sizeof(incoming)) {
    sendToAddress[0] = incoming[0];
    sendToAddress[1] = incoming[1];
    sendToAddress[2] = incoming[2];
    sendToAddress[3] = incoming[3];
    sendToAddress[4] = incoming[4];
    sprintf(message, "Address: %d, %d, %d, %d, %d", sendToAddress[0], sendToAddress[1], sendToAddress[2], sendToAddress[3], sendToAddress[4]);
    Serial.println((char*)message);
    for(int i=0;i<32;i++) {
      message[i] = incoming[5 + i];
    }
    radio.stopListening();
    radio.openWritingPipe(sendToAddress);
    radio.write(&message, sizeof(message));
    radio.startListening();

    incomingPosition = 0;
  }
}

void handleTemperature() {
  unsigned long nowMillis = millis();
  if (nowMillis < lastMillis || nowMillis - lastMillis >= 60000) {
    lastMillis = nowMillis;
    readAndSendTemperature();
  }
}

void readAndSendTemperature() {
  float temp = getTemperature();
  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.println();
  prepareMessage(temp*100);
  outputMessage();
}

void prepareMessage(float temp) {
  message[0] = SENSOR_ID;
  message[1] = SENSOR_TYPE; // temperature
  message[2] = 255; // battery % or 255 for unavailable
  message[3] = ((uint16_t)temp) >> 8;
  message[4] = ((uint16_t)temp) & 0xff;
}

float getTemperature() {
  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(0);
#ifdef DEBUG
  Serial.print(temp);
#endif
  return temp;
}
