/*
 * Door sensor
 * 
 * This is specifically for battery powered ATTiny84
 * It connects the door sensor to pin 0 (first one after Vcc)
 * 
 * The MCU is constantly sleeping.
 * Whenever the door sensor on pin 0 goes from HI to LOW,
 * the MCU powers up through Pin Change Interrupt (PCINT)
 * 
 * The ATTiny84 has PCINT on bacially all digital inputs.
 * The numbers do not match the arduino pin numbering.
 * Below shows the pin and PCINT numbers resepctively.
 * 
 *      Vcc        Gnd
 * 0 PCINT8        10 PCINT0
 * 1 PCINT9        9  PCINT1
 * - PCINT11       8  PCINT2
 * 2 PCINT10       7  PCINT3
 * 3 PCINT7        6  PCINT4
 * 4 PCINT6        5  PCINT5
 * 
 * On the ATTiny84 there are two PCINT control registers PCMSK0 and PCMSK1
 * and are enabled in GIMSK using PCIE0 or PCIE1
 * THE PCMSK0 controls PCINT0 ... PCINT7
 * The PCMSK1 controls PCINT8 ... PCINT11
 * (See http://ww1.microchip.com/downloads/en/DeviceDoc/doc8006.pdf)
 * 
 * When choosing pin to trigger wakeup, care must be taken to its numbering,
 * and if it is in PCMSK0 or PCMSK1.
 * 
 * IMPORTANT: depending on if it is PCIE0 or PCIE1, two different ISR callbacks
 * must be overridden. Otherwise the MCU will reset occasionally.
 * They are ISR(PCINT0_vect); and ISR(PCINT1_vect);
 * 
 */
#include <nRF24L01.h>
#include <RF24.h>

#include <avr/sleep.h>
#include <avr/interrupt.h>

// SENSOR CONFIG
#define SENSOR_ID 102
#define SENSOR_TYPE 2

// #define DEBUG

// DOOR PIN
#define DOOR_PIN 0
#define DOOR_PCINT PCINT8
#define DOOR_PCMSKn PCMSK1
#define DOOR_PCIE PCIE1

// RADIO
// ARDUINO is 9 and 8
// ATTINY84 is 8 and 7
#define CE_PIN  8
#define CSN_PIN 7
RF24 radio(CE_PIN, CSN_PIN);

byte gateway[5] = "ABCDE";

ISR(PCINT1_vect) {
}

ISR(PCINT0_vect) {
}

void setup() {
#ifdef DEBUG
  Serial.begin(115200);
  Serial.println("Starting");
#endif

  pinMode(DOOR_PIN, INPUT_PULLUP);

  // disable ADC
  ADCSRA &= ~_BV(ADEN);

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

  radio.powerDown();
  delay(50);
  sleep();
  radio.powerUp();
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

void sleep() {
  GIMSK |= _BV(DOOR_PCIE);   // Enable Pin Change Interrupts
  DOOR_PCMSKn |= _BV(DOOR_PCINT); // Select interrupt pin

  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
  sleep_enable();
  sei();
  sleep_cpu();                        // System sleeps here
  cli();
  DOOR_PCMSKn &= ~_BV(DOOR_PCINT);
  sleep_disable();                     // System continues execution here when watchdog timed out 
  sei();
}
