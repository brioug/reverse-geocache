/*
  DisplayLine.h - Library for managing lines of text on an LCD screen. Long text lines will scroll.
  Created by Jean-Baptiste Tréglos, February 23, 2016.
*/

#ifndef DisplayLine_h
#define DisplayLine_h

#include "Arduino.h"
#include <LiquidCrystal.h>

class DisplayLine {
  private:
  static const byte NB_CHARS = 16;

  LiquidCrystal* _lcd;
  String _text;
  byte _line;
  unsigned long _scroll_delay;

  unsigned long _previousTime;
  byte _text_offset;
  unsigned int _text_length;
  boolean _do_scroll;

  public:
  DisplayLine();
  void Setup(LiquidCrystal* lcd, String text, byte line=0, unsigned long scroll_delay=300);
  void Update();
  void Change(String new_text);
};

#endif
