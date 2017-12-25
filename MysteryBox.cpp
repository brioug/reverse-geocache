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

void MysteryBox::Setup(Place place, LiquidCrystal* lcd) {
  _place = place;
  _lcd = lcd;
  _l1.Setup(lcd, "");
  _l2.Setup(lcd, "", 1);
  
  setPlace(place);
}

void MysteryBox::ReprogramSetup(LiquidCrystal *lcd) {
  _lcd = lcd;
  lcdPowerOn();
  _l1.Setup(lcd, "REPROGRAM MODE");
  _l2.Setup(lcd, "No sync...", 1);
}

void MysteryBox::setPlace(Place place) {
  _place = place;
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
        _l1.Change("Le cadeau d'Hugo");
        _l2.Change("   Pour Noel !  ");
        stateIsOldNow();
      } else if (waited(4000)) {
        changeState(BOX_STATE_SHUTDOWN);
      }
      break;
      
    case BOX_STATE_INIT:
      if (_state_just_changed) {
        lcdPowerOn();
        _l1.Change("Salut Hugo !");
        _l2.Change("Essai " + String(_current_try) + "/" + String(NB_TRIES));
        stateIsOldNow();
      } else if (waited(4000)) {
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
        _l1.Change(F("Pas de signal GPS !"));
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
          _place.lat,
          _place.lng
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
        _l1.Change(F("Tu y es ! Ton cadeau n'est pas loin..."));
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

void MysteryBox::ReprogramUpdate(byte test_pin) {
  Place new_place;
  float new_lat;
  float new_lng;
  
  if (millis() >= _previousTime + 1000) {
    _previousTime = millis();
    if (_new_place_saved == false) {
      if (_gps.location.isValid()) {
        new_lat = _gps.location.lat();
        new_lng = _gps.location.lng();
        _l1.Change(String(new_lat, 6) + " / " + String(_gps.satellites.value()));
        _l2.Change(String(new_lng, 6) + " / " + String(_gps.hdop.value()));
  
        if (digitalRead(test_pin) == LOW) {
          new_place = { new_lat, new_lng };
          EEPROM.put(0x0, new_place);
          _l1.Change("Position saved");
          _l2.Change("");
          _new_place_saved = true;
        }
      }
    }

    _l1.Update();
    _l2.Update();
  }
  
  while (Serial.available() > 0) {
    _gps.encode(Serial.read());
  }
}

