#include "Blinker.h"
#include "Arduino.h"

#define OFF_BLINK_TIMER  1900
#define ON_BLINK_TIMER   100

Blinker::Blinker(char ledPin)
{
  _ledPin = ledPin;
  _currentState = LOW;
  pinMode(_ledPin, OUTPUT);
}

void Blinker::blink(void)
{
  char stateChanged = 0;
  unsigned long currentDate = millis();
  
  if (_currentState == LOW && currentDate - _lastTimeSwitch > OFF_BLINK_TIMER) {
    _currentState = HIGH;
    stateChanged = 1;
  } else if (_currentState == HIGH && currentDate - _lastTimeSwitch > ON_BLINK_TIMER) {
    _currentState = LOW;
    stateChanged = 1;
  }
  if (stateChanged == 1) {
    _lastTimeSwitch = currentDate;
    digitalWrite(_ledPin, _currentState);
  }
}

