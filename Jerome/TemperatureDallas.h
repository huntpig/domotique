#include "OneWire.h"

typedef enum {
    TemperatureDallasStateReset,
    TemperatureDallasStateResetWait,
} TemperatureDallasState;

typedef enum {
    TemperatureDallasTypeDS18S20 = 0x10,
    TemperatureDallasTypeDS18B20 = 0x28,
    TemperatureDallasTypeDS1822 = 0x22,
} TemperatureDallasType;

class OneWire;

class TemperatureDallas
{
    OneWire                     *_oneWire;
    byte                        _address[8];
    TemperatureDallasState      _state;
    unsigned long               _timer;
    float                       _celsius;

public:
    TemperatureDallas(byte adresse[8], OneWire *oneWire);
    
    float celsius(void) { return _celsius; };
    const char *typeName(void);
    const char *addressString(void);
    void loop(void);
};
