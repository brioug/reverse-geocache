/*
  MysteryBox.h - Library for reverse geocache "mystery" box.
  Created by Jean-Baptiste Tréglos, February 23, 2016.
*/

#ifndef MysteryBox_h
#define MysteryBox_h

#include "Arduino.h"
#include <avr/sleep.h>
#include <TinyGPS++.h>
#include <Servo.h>
#include "DisplayLine.h"
#include "Team.h"

void wakeUpNow();
void sleepNow();

class MysteryBox {
  private:
  // Behavior constants
  static const byte NB_ACQU_TRIES = 10;
  static const byte NB_TRIES = 3;
  static const unsigned int MAIN_DELAY_DURATION = 4000;

  // State constants
  static const byte BOX_STATE_START_UP = 0;
  static const byte BOX_STATE_INIT = 1;
  static const byte BOX_STATE_NO_MORE_TRIES = 2;
  static const byte BOX_STATE_TRIES_REMAINING = 3;
  static const byte BOX_STATE_NO_SYNC = 4;
  static const byte BOX_STATE_IN_SYNC = 5;
  static const byte BOX_STATE_UNLOCKED = 6;
  static const byte BOX_STATE_TOO_FAR = 7;
  static const byte BOX_STATE_SHUTDOWN = 8;
  static const byte BOX_STATE_THE_END = 9;

  // State machine internal state
  TinyGPSPlus _gps;
  DisplayLine _l1, _l2;
  Servo _servo;
  LiquidCrystal* _lcd;

  Team _team;
  byte _state;
  unsigned long _previousTime;
  byte _current_try;
  byte _acqu_try;
  boolean _state_just_changed;
  byte _servo_pin;
  byte _cheat_pin;
  unsigned long _distance;

  public:
  boolean sleeping;
  
  MysteryBox();
  void Setup(Team _team, LiquidCrystal* lcd, byte servo_pin, byte cheat_pin);
  void changeState(byte new_state);
  void stateIsOldNow();
  void lockLatch();
  void unlockLatch();
  void lcdPowerOn();
  void lcdPowerOff();
  boolean waited(unsigned int wait_time=MAIN_DELAY_DURATION);
  void Update();
  void changeTeam(Team _team);
};

#endif
