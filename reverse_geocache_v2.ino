#include "MysteryBox.h"
#include <LiquidCrystal.h>

// Hardware constants
static const uint32_t GPS_BAUD = 9600;
static const byte TEST_PIN = 2;
static const byte CHEAT_PIN = 4;
static const byte POWER_OFF_PIN = 3;

// Teams
Team teams[] = {
  { "Les Ragondins", 48.656335, -1.970385 },
  { "Ze Sprountz team", 48.618890, -2.105556 },
  { "Gluth World Co. Inc.", 48.656335, -1.970385 },
  { "Les blaireaux", 48.656335, -1.970385 }
};

// Global resource objects
TinyGPSPlus gps;
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// Main state machine
MysteryBox box;

byte toTeamIndex() {
  if (analogRead(1) <= 50) {
    return 0;
  } else if (analogRead(2) <= 50) {
    return 1;
  } else if (analogRead(3) <= 50) {
    return 2;
  } else if (analogRead(4) <= 50) {
    return 3;
  } else { // Comportement par défaut
    return 0;
  }
}

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
  pinMode(POWER_OFF_PIN, OUTPUT);
  digitalWrite(POWER_OFF_PIN, HIGH);
  pinMode(TEST_PIN, INPUT_PULLUP);
  pinMode(CHEAT_PIN, INPUT_PULLUP);
  lcd.begin(16, 2);
  box.Setup(teams[toTeamIndex()], &lcd, CHEAT_PIN);
}

void loop() {
  box.Update();
}

