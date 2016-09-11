#include "MysteryBox.h"
#include <LiquidCrystal.h>

// Hardware constants
static const uint32_t GPS_BAUD = 9600;
static const byte TEST_PIN = 2;
static const byte RESET_PIN = 3;
static const byte DEBUG_MODE = 100;

// Teams
Team teams[] = {
  { 48.695545, -1.888100 }, // Point "Champ vers panneau routier"
  { 48.695114, -1.884603 }, // Point "Petit point d'eau près du parking"
  { 48.696461,-1.882665 }, // Point "Le Verger"
  { 48.6988400, -1.888726 }, // Point "Petit chemin entre bout du champs et chemin de ronde"
};

// Global resource objects
TinyGPSPlus gps;
LiquidCrystal lcd(8, 9, 10, 11, 12, 13);
int team_index;

// Main state machine
MysteryBox box;

byte toTeamIndex() {
  if (analogRead(2) <= 50) {
    return 2; // Team index in teams[] array
  } else { // Mode debug : Affichage des coordonnées
    return DEBUG_MODE;
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
  pinMode(TEST_PIN, INPUT_PULLUP);
  pinMode(RESET_PIN, INPUT_PULLUP);
  lcd.begin(16, 2);
  team_index = toTeamIndex();
  
  if (team_index == DEBUG_MODE) { // Mode debug
    box.DebugSetup(&lcd);
  } else { // Normal mode
    box.Setup(teams[team_index], &lcd);
  }
}

void loop() {
  if (digitalRead(RESET_PIN) == 0) {
    soft_restart();
  }
  if (team_index == DEBUG_MODE) { // Mode debug
    box.DebugUpdate();
  } else {
    box.Update();
  }
}

