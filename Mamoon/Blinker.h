
class Blinker
{
public:
  Blinker(char ledPin);
  
  void blink(void);
  
protected:
  char _ledPin;
  unsigned long _lastTimeSwitch;
  int _currentState;
};

