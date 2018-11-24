#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#include <printf.h>

#define CE_PIN   9
#define CSN_PIN 8

RF24 radio(CE_PIN,CSN_PIN);

byte address[5] = "ABCDE";
byte sendToAddress[5] = "EDCBA";

void setup() {
  Serial.begin(115200);
  printf_begin();
  Serial.println("Start receiving");

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

void loop() {
  if (radio.available()) {
    radio.read(&message, sizeof(message));
    radio.flush_tx();
    radio.stopListening();
    Serial.print("Data ");
    formOutput();
    Serial.println(output);
    radio.startListening();
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
