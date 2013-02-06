#include "Arduino.h"
#include "TemperatureDallas.h"

TemperatureDallas::TemperatureDallas(byte address[8], OneWire *oneWire)
{
    _oneWire = oneWire;
    memcpy(_address, address, sizeof(_address));
    _state = TemperatureDallasStateReset;
    _celsius = 20.0;
}

static byte dallasType(byte address[12])
{
    switch (address[0]) {
        case TemperatureDallasTypeDS18S20:
            return 1;
            break;
        case TemperatureDallasTypeDS18B20:
        case TemperatureDallasTypeDS1822:
            return 0;
            break;
        default:
            return -1;
    } 
}

const char *TemperatureDallas::typeName(void)
{
    const char *result = NULL;
    
    switch(_address[0]) {
        case TemperatureDallasTypeDS18S20:
            result = "DS18S20";
            break;
        case TemperatureDallasTypeDS18B20:
            result = "DS18B20";
            break;
        case TemperatureDallasTypeDS1822:
            result = "DS1822";
            break;
    }
    return result;
}

const char *TemperatureDallas::addressString(void)
{
    char *string;
    char *cursor;
    char ii;
    
    cursor = string = (char *)malloc(24);
    for(ii = 0; ii < 8; ii++) {
        sprintf(cursor, "%02x", _address[ii]);
        cursor += 2;
        cursor[0] = ':';
        cursor++;
    }
    cursor--;
    cursor[0] = '\0';
    return string;
}

void TemperatureDallas::loop()
{
    switch(_state) {
        case TemperatureDallasStateReset:
            _oneWire->reset();
            _oneWire->select(_address);    
            _oneWire->write(0x44,1);         // Read Scratchpad
            _state = TemperatureDallasStateResetWait;
            _timer = millis();
            break;
        case TemperatureDallasStateResetWait:
            if (_timer > millis()) {
                _timer = millis();
            } else if (_timer + 1000 < millis()) {
                byte ii;
                byte data[12];
                
                _oneWire->reset();
                _oneWire->select(_address);    
                _oneWire->write(0xBE);         // Read Scratchpad
                for (ii = 0; ii < 9; ii++) {           // we need 9 bytes
                    data[ii] = _oneWire->read();
                }
                unsigned int raw = (data[1] << 8) | data[0];
                if (dallasType(_address)) {
                    raw = raw << 3; // 9 bit resolution default
                    if (data[7] == 0x10) {
                        // count remain gives full 12 bit resolution
                        raw = (raw & 0xFFF0) + 12 - data[6];
                    }
                } else {
                    byte cfg = (data[4] & 0x60);
                    if (cfg == 0x00) raw = raw << 3;  // 9 bit resolution, 93.75 ms
                    else if (cfg == 0x20) raw = raw << 2; // 10 bit res, 187.5 ms
                    else if (cfg == 0x40) raw = raw << 1; // 11 bit res, 375 ms
                    // default is 12 bit resolution, 750 ms conversion time
                }
                _celsius = (float)raw / 16.0;
                _state = TemperatureDallasStateReset;
            }
            break;
    }
}
