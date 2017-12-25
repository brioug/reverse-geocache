#include "MysteryBox.h"
#include <LiquidCrystal.h>
#include <EEPROM.h>

/*
 * TODO:
 *  1. Store coordinates in EEPROM
 *  2. Implement reprogram mode
 *  3. Change messages
 *  4. Change timings & Workflow
 *  5. Add quality of GPS signal indicator ?
 */

// Hardware constants
static const uint32_t GPS_BAUD = 9600;
static const byte TEST_PIN = 2;
static const byte MODE_PIN = 4;
static const byte NORMAL_MODE = 1;
static const byte REPROGRAM_MODE = 2;

// Global resource objects
Place place;
TinyGPSPlus gps;
LiquidCrystal lcd(8, 9, 10, 11, 12, 13);
byte running_mode = NORMAL_MODE;

// Main state machine
MysteryBox box;

void sleepNow() {
  box.sleeping = true;
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  attachInterrupt(digitalPinToInterrupt(TEST_PIN), wakeUpNow, LOW);
  sleep_mode();

  // Wakes up here
  if (!box.sleeping) {
    detachInterrupt(digitalPinToInterrupt(TEST_PIN));
    sleep_disable();
  }
}

void wakeUpNow() {
  box.sleeping = false;
}

void setup() {
  Serial.begin(GPS_BAUD);
  pinMode(TEST_PIN, INPUT_PULLUP);
  pinMode(MODE_PIN, INPUT_PULLUP);
  lcd.begin(16, 2);
  
  if (digitalRead(MODE_PIN) == HIGH) {
    running_mode = NORMAL_MODE;
  } else {
    running_mode = REPROGRAM_MODE;
  }
  
  if (running_mode == REPROGRAM_MODE) {
    box.ReprogramSetup(&lcd);
  } else {
    EEPROM.get(0x0, place);
    box.Setup(place, &lcd);
  }
}

void loop() {
  if (running_mode == REPROGRAM_MODE) {
    box.ReprogramUpdate(TEST_PIN);
  } else {
    box.Update();
  }
}

