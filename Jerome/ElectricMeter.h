#include <Arduino.h>

#ifndef __ElectricMeter__
#define __ElectricMeter__

class ElectricMeter;

typedef void (*ElectricMeterChecksumErrorCallback)(ElectricMeter *electricMeter, const String *currentLine);
typedef void (*ElectricMeterUnknownLineCallback)(ElectricMeter *electricMeter, const String *currentLine);

typedef enum {
    ElectricMeterStateWaitingForCycle,
    ElectricMeterStateInCycle,
    ElectricMeterStateCycleDone
} ElectricMeterState;

class ElectricMeter
{
    HardwareSerial                      *_serialPort;
    unsigned char                       _currentMeterPin;
    ElectricMeterState                  _stateCycle;
    String                              _line;
    ElectricMeterChecksumErrorCallback  _checksumErrorCallback;
    ElectricMeterUnknownLineCallback    _unknownLineCallback;
    
    String                              _currentCycle;
    String                              _completeCycle;
    
    char parseLine(void);    
    
public:
    ElectricMeter(void);
    
    void setSerialPort(HardwareSerial *serialPort);
    void addMeterPin(unsigned char meterPin) { pinMode(meterPin, OUTPUT); digitalWrite(meterPin, LOW); };
    void enableMeter(unsigned char pin) { digitalWrite(_currentMeterPin, LOW); _currentMeterPin = pin; digitalWrite(_currentMeterPin, HIGH); };
    void setChecksumErrorCallback(ElectricMeterChecksumErrorCallback checksumErrorCallback) { _checksumErrorCallback = checksumErrorCallback; };
    void setUnknownLineCallback(ElectricMeterUnknownLineCallback unknownLineCallback) { _unknownLineCallback = unknownLineCallback; };
    char listenPort(void);
    
    ElectricMeterState stateCycle(void) { return _stateCycle; };
    void resetStateCycle(void) { _stateCycle = ElectricMeterStateWaitingForCycle; };
    String data(void) { return _completeCycle; };
    
};

/* __ElectricMeter__ */
#endif
