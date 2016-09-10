/*
  MysteryBox.cpp - Library for reverse geocache "mystery" box.
  Created by Jean-Baptiste Tréglos, February 23, 2016.
*/

#include "MysteryBox.h"

MysteryBox::MysteryBox() {
  _acqu_try = 0;
  _distance = 0;
  sleeping = false;
}

void MysteryBox::Setup(Team team, LiquidCrystal* lcd) {
  _team = team;
  _lcd = lcd;
  _l1.Setup(lcd, "");
  _l2.Setup(lcd, "", 1);
  
  changeTeam(team);
}

void MysteryBox::DebugSetup(LiquidCrystal *lcd) {
  _lcd = lcd;
  lcdPowerOn();
  _l1.Setup(lcd, "DEBUG MODE");
  _l2.Setup(lcd, "No sync...", 1);
}

void MysteryBox::changeTeam(Team team) {
  _team = team;
  _current_try = 1;

  changeState(BOX_STATE_START_UP);
}

void MysteryBox::changeState(byte new_state) {
  _state = new_state;
  _state_just_changed = true;
  _previousTime = millis();
}

void MysteryBox::stateIsOldNow() {
  _state_just_changed = false;
}

boolean MysteryBox::waited(unsigned int wait_time) {
  return (millis() - _previousTime) > wait_time;
}

void MysteryBox::lcdPowerOn() {
  _lcd->display();
}

void MysteryBox::lcdPowerOff() {
  _lcd->noDisplay();
}

void MysteryBox::Update() {
  switch (_state) {
    case BOX_STATE_START_UP:
      if (_state_just_changed) {
        lcdPowerOn();
        _l1.Change("** MysteryBox **");
        _l2.Change("      v1.0      ");
        stateIsOldNow();
      } else if (waited(4000)) {
        changeState(BOX_STATE_SHUTDOWN);
      }
      break;
      
    case BOX_STATE_INIT:
      if (_state_just_changed) {
        lcdPowerOn();
        _l1.Change("Bonjour !");
        _l2.Change("Essai " + String(_current_try) + "/" + String(NB_TRIES));
        stateIsOldNow();
      } else if (waited(6000)) {
        if (_current_try > NB_TRIES) {
          changeState(BOX_STATE_NO_MORE_TRIES);
        } else {
          changeState(BOX_STATE_TRIES_REMAINING);
        }
      }
      break;

    case BOX_STATE_NO_MORE_TRIES:
      if (_state_just_changed) {
        _l1.Change(F("**  PERDU !!  **"));
        stateIsOldNow();
      } else if (waited()) {
        changeState(BOX_STATE_THE_END);
        _l1.Change(F("**  PERDU !!  **"));
        _l2.Change(F(" "));
      }
      break;

    case BOX_STATE_TRIES_REMAINING:
      if (_state_just_changed) {
        _l1.Change(F("Recherche GPS..."));
        stateIsOldNow();
      } else if (waited(2000)) {
        if (_gps.location.isValid()) {
          _acqu_try = 0;
          changeState(BOX_STATE_IN_SYNC);
        } else {
          if (_acqu_try < NB_ACQU_TRIES) {
            _acqu_try++;
            _previousTime = millis();
          } else {
            changeState(BOX_STATE_NO_SYNC);
          }
        }
      }
      break;

    case BOX_STATE_NO_SYNC:
      if (_state_just_changed) {
        _l1.Change(F("Pas de signal GPS :("));
        _acqu_try = 0;
        stateIsOldNow();
      } else if (waited()) {
        changeState(BOX_STATE_SHUTDOWN);
      }
      break;

    case BOX_STATE_IN_SYNC:
      if (_state_just_changed) {
        _l1.Change(F("... Trouve !"));
        stateIsOldNow();
      } else if (waited(2000)) {
        // Test distance
        _distance = (unsigned long)TinyGPSPlus::distanceBetween(
          _gps.location.lat(),
          _gps.location.lng(),
          _team.lat,
          _team.lng
        );
        if (_distance <= 2) {
          changeState(BOX_STATE_UNLOCKED);
        } else {
          changeState(BOX_STATE_TOO_FAR);
        }
      }
      break;

    case BOX_STATE_UNLOCKED:
      if (_state_just_changed) {
        _l1.Change(F("**  GAGNE !!  **"));
        stateIsOldNow();
      }
      break;

    case BOX_STATE_TOO_FAR:
      if (_state_just_changed) {
        _l1.Change("Distance " + String(_distance) + " m. Trop Loin !");
        _current_try++;
        stateIsOldNow();
      } else if (waited(6000)) {
        changeState(BOX_STATE_SHUTDOWN);
      }
      break;

    case BOX_STATE_SHUTDOWN:
      if (_state_just_changed) {
        lcdPowerOff();
        stateIsOldNow();
        sleepNow();
      } else {
        changeState(BOX_STATE_INIT);
      }
      break;

    case BOX_STATE_THE_END:
      if (_state_just_changed) {
        lcdPowerOff();
        stateIsOldNow();
        Serial.end();
      }
  }

  _l1.Update();
  _l2.Update();
  while (Serial.available() > 0) {
    _gps.encode(Serial.read());
  }
}

void MysteryBox::DebugUpdate() {
  if (millis() >= _previousTime + 500) {
    _previousTime = millis();
    if (_gps.location.isValid()) {
      _l1.Change(String(_gps.location.lat(), 6) + " / " + String(_gps.satellites.value()));
      _l2.Change(String(_gps.location.lng(), 6) + " / " + String(_gps.hdop.value()));
    }

    _l1.Update();
    _l2.Update();
  }
  
  while (Serial.available() > 0) {
    _gps.encode(Serial.read());
  }
}

