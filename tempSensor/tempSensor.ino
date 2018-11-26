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
#ifdef ATTINY_CORE
#define ONE_WIRE_BUS 9
#else
#define ONE_WIRE_BUS 2
#endif

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// SENSOR CONFIG
#define SENSOR_ID 100
#define SENSOR_TYPE 1

// #define DEBUG

// RADIO
// ARDUINO is 9 and 8
// ATTINY84 is 8 and 7
#ifdef ATTINY_CORE
#define CE_PIN  8
#define CSN_PIN 7
#else
#define CE_PIN  9
#define CSN_PIN 8
#endif

RF24 radio(CE_PIN,CSN_PIN);

byte gateway[5] = "ABCDE";

// sleep
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>

void setup_watchdog(int ii) {
  // 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
  // 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec

  uint8_t bb;
  if (ii > 9 ) ii=9;
  bb=ii & 7;
  if (ii > 7) bb|= (1<<5);
  bb|= (1<<WDCE);

  MCUSR &= ~(1<<WDRF);
  // start timed sequence
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  // set new watchdog timeout value
  WDTCSR = bb;
  WDTCSR |= _BV(WDIE);
}

void system_sleep() {
  setup_watchdog(9);                   // approximately 8 seconds sleep
 
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
  sleep_enable();
  sei();                               // Enable the Interrupts so the wdt can wake us up

  sleep_mode();                        // System sleeps here

  sleep_disable();                     // System continues execution here when watchdog timed out 
}

ISR(WDT_vect) {}

void setup() {
#ifdef DEBUG
  Serial.begin(115200);
  Serial.println("Starting");
#endif

  sensors.begin();

  radio.begin();
  radio.setDataRate( RF24_250KBPS );
  radio.setPALevel(RF24_PA_HIGH);
  radio.setRetries(15,15);

  radio.openWritingPipe(gateway);
}

uint8_t message[32];
int counter = 5;

void loop() {
  counter++;
  if (counter >= 6) {
    radio.powerUp();
    prepareMessage(getTemperature()*100);
    sendMessage();
    counter = 0;

    radio.powerDown();
    delay(50);
  }

  system_sleep();
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
