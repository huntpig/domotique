  /*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
 
  This example code is in the public domain.
 */

#include <Wire.h>
#include <OneWire.h>
#include <DallasSensor.h>
#include <SunblindDriver.h>
#include <Adafruit_BMP085.h>
#include <DHTSensor.h>
#include <SensorList.h>
#include <BMP085Sensor.h>
#include <RF12.h>
#include <JET.h>
#include <SPI.h>

#define RFM12B_NODE_ID        1

#define SUNBLIND_UP_PIN       5
#define SUNBLIND_STOP_PIN     4
#define SUNBLIND_DOWN_PIN     3
#define ONE_WIRE_PIN          6
#define DHT_PIN               7

#define SERIAL_SPEED          115200
#define LOOP_DELAY            5000

OneWire           oneWire(ONE_WIRE_PIN);
SensorList        sensorList;

SunblindDriver    sunblindDriver(SUNBLIND_UP_PIN, SUNBLIND_STOP_PIN, SUNBLIND_DOWN_PIN);

static unsigned long lastTime = 0;

// the setup routine runs once when you press reset:
void setup()
{
    uint8_t address[8];
    Serial.begin(SERIAL_SPEED);
    Serial.print("rf12 init ");
    Serial.println(rf12_initialize(RFM12B_NODE_ID, RF12_868MHZ));
    while (oneWire.find_address(address)) {
        if (DallasSensor::sensorType(address)) {
          sensorList.addSensor(new DallasSensor(address, &oneWire));
        }
    }
    sensorList.addSensor(new BMP085Sensor());
    sensorList.addSensor(new DHTSensor(DHT_PIN, DHTSensorType_AM2301));
    sensorList.begin();
    Serial.println("done");
}

char command[255];
unsigned int index = 0;

// the loop routine runs over and over again forever:
void loop()
{
    transfertRF12DataToStream(&Serial);
    sensorList.loop();
    sunblindDriver.loop();
    if (lastTime > millis()) {
        lastTime = millis();
    } else if (lastTime + LOOP_DELAY < millis()) {
        sensorList.readValues();
        sensorList.printInfo(&Serial, RFM12B_NODE_ID);
        Serial.println("--");
        lastTime = millis();
    }
    if (Serial.available() > 0) {
        char a;
        boolean commandDone = false;
        
        a = Serial.read();
        if (a == '\n' || a == '\r') {
            commandDone = true;
        } else {
            command[index] = a;
            index++;
            command[index] = 0;
        }
        if (commandDone || index == sizeof(command)) {
            if (strcmp(command, "up") == 0) {
                sunblindDriver.sendUp();
            } else if (strcmp(command, "down") == 0) {
                sunblindDriver.sendDown();
            } else if (strcmp(command, "stop") == 0) {
                sunblindDriver.sendStop();
            } else if (strcmp(command, "zob") == 0) {
                Serial.println("zob toi meme");
            } else if (command[0] == 0) {
            } else {
                Serial.print("Error: ");
                Serial.println(command);
            }
            index = 0;
            command[index] = 0;
        }
    }
}
