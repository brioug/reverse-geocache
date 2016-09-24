/*
  DisplayLine.cpp - Library for managing lines of text on an LCD screen. Long text lines will scroll.
  Created by Jean-Baptiste Tréglos, February 23, 2016.
*/

#include "DisplayLine.h"

DisplayLine::DisplayLine() { }

void DisplayLine::Setup(LiquidCrystal* lcd, String text, byte line, unsigned long scroll_delay) {
  _lcd = lcd;
  _line = line;
  _scroll_delay = scroll_delay;
  _previousTime = millis();

  Change(text);
}

void DisplayLine::Update() {
  if (_do_scroll) {
    if ((millis() - _previousTime) > _scroll_delay) {
      _previousTime = millis();
      _lcd->setCursor(0, _line);
      _lcd->print((_text+"    "+_text).substring(_text_offset, _text_offset+NB_CHARS));
      _text_offset++;
      if (_text_offset >= _text_length) {
        _text_offset = 0;
      }
    }
  } else {
    _lcd->setCursor(0, _line);
    _lcd->print(_text);
  }
}

void DisplayLine::Change(String new_text) {
  _text = new_text;
  _text_offset = 0;
  _text_length = new_text.length();
  _do_scroll = (_text_length > NB_CHARS);

  // Clear line
  String tmp = " ";
  for(int i=1; i<NB_CHARS; i++) {
    tmp += " ";
  }
  _lcd->setCursor(0, _line);
  _lcd->print(tmp);
}
