/*
  Web Server
 
  This sketch acts as a server using a RedFly-Shield. 
 */

#include <Arduino.h>
#include "ElectricMeter.h"
#include <DallasSensor.h>
#include <DHTSensor.h>
#include <SensorList.h>
#include <BMP085Sensor.h>
#include <RF12.h>
#include <JET.h>
#include <SerialCommand.h>

#define RFM12B_NODE_ID      1
#define RFM12B_NODE_ID_STR  "1"

#define COMPTEUR1           5
#define COMPTEUR2           6
#define DHT_PIN             3
#define ONE_WIRE_PIN        8

ElectricMeter               electricMeter( RFM12B_NODE_ID_STR " EDF ");
SensorList                  sensorList;
OneWire                     ds(ONE_WIRE_PIN);
SerialCommand               serialCommand;

void setup()
{
    uint8_t address[8];
    
    Serial.begin(SERIAL_SPEED);
    Serial.print("rf12 init ");
    Serial.println(rf12_initialize(RFM12B_NODE_ID, RF12_868MHZ));
    serialCommand.setStream(&Serial);
    
    electricMeter.addMeterPin(COMPTEUR1);
    electricMeter.addMeterPin(COMPTEUR2);
    electricMeter.enableMeter(COMPTEUR1);
    electricMeter.setSerialPort(&Serial1);
    
    while (ds.find_address(address)) {
        sensorList.addSensor(new DallasSensor(address, &ds));
    }
    sensorList.addSensor(new DHTSensor(DHT_PIN, DHTSensorType_22));
    sensorList.addSensor(new BMP085Sensor());
    sensorList.begin();
    Serial.println("ok");
}

int mytimer = 0;

void loop()
{
    uint8_t sock;
    char buf[512], *ptr;
    uint16_t buf_len, rd, len;
    
    transfertRF12DataToStream(&Serial);
    sensorList.loop();
    if (serialCommand.loop()) {
        if (strcmp(serialCommand.getCommand(), "zob") == 0) {
            Serial.println("zob-toi-meme");
            serialCommand.purgeCommand();
        } else if (strcmp(serialCommand.getCommand(), "munin-values") == 0) {
            rf12_sendStart(0, "munin-values", strlen("munin-values"));
        }
        serialCommand.purgeCommand();
    }
    //check if socket is opened
    if (electricMeter.listenPort()) {
        char buffer[32];
        
        Serial.print(electricMeter.data());
        sensorList.readValues();
        sensorList.printInfo(&Serial, RFM12B_NODE_ID);
        Serial.println("--");
    }
}
