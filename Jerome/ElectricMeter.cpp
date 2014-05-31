#include "ElectricMeter.h"


#define IDENTIFICATION_CMD                          "ADCO "
#define OPTION_TARIFAIRE_CMD                        "OPTARIF "
#define INTESITE_SOUSCRITE_CMD                      "ISOUSC "
#define BASE_CMD                                    "BASE "
#define INDEX_HEURES_CREUSE_CMD                     "HCHC "
#define INDEX_HEURES_PLEINES_CMD                    "HCHP "
#define INDEX_HEURES_NORMALES_CMD                   "EJPHN "
#define INDEX_HEURES_POINTE_MOBILE_CMD              "EJPHPM "
#define INDEX_HEURES_CREUSES_JOURS_BLEUS_CMD        "BBRHCJB "
#define INDEX_HEURES_PLEINES_JOURS_BLEUS_CMD        "BBRHPJB "
#define INDEX_HEURES_CREUSES_JOURS_BLANCS_CMD       "BBRHCJW "
#define INDEX_HEURES_PLEINES_JOURS_BLANCS_CMD       "BBRHPJW "
#define INDEX_HEURES_CREUSES_JOURS_ROUGES_CMD       "BBRHCJR "
#define INDEX_HEURES_PLEINES_JOURS_ROUGES_CMD       "BBRHPJR "
#define PREAVIS_EJP_CMD                             "PEJP "
#define PERIODE_TARIFAIRE_EN_COURS_CMD              "PTEC "
#define INTENSITE_INSTANTANEE_CMD                   "IINST "
#define AVERTISSEMENT_DEPASSEMENT_CMD               "ADPS "
#define INTENSITE_MAXIMALE_CMD                      "IMAX "
#define PUISSANCE_APPARENTE_CMD                     "PAPP "
#define GROUPE_HORAIRE_CMD                          "HHPHC "
#define MODE_ETAT_CMD                               "MOTDETAT "

#define COPY_VALUE(field, command) _line.getBytes(field, sizeof(field), sizeof(command))

static char checksum(String *line)
{
    unsigned int ii, length;
    char sum = 0;
    
    length = line->length() - 2;
    for (ii = 0; ii < length; ii++) {
        sum = sum + (*line)[ii];
    }
    sum = (sum & 0x3F) + 0x20;
    return sum;
}

ElectricMeter::ElectricMeter(char *linePrefix)
{
    _linePrefix = linePrefix;
    _serialPort = NULL;
    _stateCycle = ElectricMeterStateWaitingForCycle;
    _checksumErrorCallback = NULL;
    _unknownLineCallback = NULL;
}

void ElectricMeter::setSerialPort(HardwareSerial *serialPort)
{
    _serialPort = serialPort;
    _serialPort->begin(1200);
    if (_serialPort == &Serial) {
        UCSR0C = B00100100;
    } else if (_serialPort == &Serial1) {
        UCSR1C = B00100100;
    } else if (_serialPort == &Serial2) {
        UCSR2C = B00100100;
    } else if (_serialPort == &Serial3) {
        UCSR3C = B00100100;
    }
}

char ElectricMeter::listenPort(void)
{
    char lineComplete = 0;
    
    if (_stateCycle == ElectricMeterStateCycleDone) {
        _stateCycle = ElectricMeterStateWaitingForCycle;
    }
    while (_serialPort->available() > 0) {
        unsigned char c;
        
        c = _serialPort->read();
        if (c == '\n' || c == '\r') {
            lineComplete = 1;
            break;
        } else {
            _line += (char)c;
        }
    }
    
    if (lineComplete) {
        if (_line.length() <= 2) {
        } else if (checksum(&_line) == _line[_line.length() - 1]) {
            if (this->parseLine()) {
                switch(_stateCycle) {
                    case ElectricMeterStateWaitingForCycle:
                        _stateCycle = ElectricMeterStateInCycle;
                        break;
                    case ElectricMeterStateInCycle:
                        _stateCycle = ElectricMeterStateCycleDone;
                        break;
                    case ElectricMeterStateCycleDone:
                        break;
                }
            }
        } else if (_checksumErrorCallback) {
            _checksumErrorCallback(this, &_line);
        }
        _line = "";
    }
    return _stateCycle == ElectricMeterStateCycleDone;
}

char ElectricMeter::parseLine(void)
{
    char identificationLine = 0;
    
    if (_line.startsWith(IDENTIFICATION_CMD)) {
        _completeCycle = _currentCycle;
        _currentCycle = "";
        identificationLine = 1;
    }
    if (_linePrefix) {
        _currentCycle += _linePrefix;
    }
    _currentCycle += _line;
    _currentCycle += "\r\n";
    return identificationLine;
}
