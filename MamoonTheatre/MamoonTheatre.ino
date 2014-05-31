/*
  Web Server
 
 A simple web server that shows the value of the analog input pins.
 using an Arduino Wiznet Ethernet shield. 
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Analog inputs attached to pins A0 through A5 (optional)
 
 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe
 
 */

#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>
#include <DHTSensor.h>
#include <AnalogSensor.h>
#include <SensorList.h>
#include <BlinkerDriver.h>
#include <RF12.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0D, 0x5C, 0xDD };

// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
#define SERVER_PORT      10000
#define NODE_ID          2
#define DHT_PIN          8
#define LED_PIN          9
#define LIGHT_PIN        A0
#define HEATER_PIN       A1
#define GROUND_PIN       A2
#define ETHERNET_ENABLED 1

BlinkerDriver blinker(LED_PIN);
EthernetServer server(SERVER_PORT);
char ethernetEnabled = ETHERNET_ENABLED;
SensorList sensorList;

void setup()
{
    IPAddress localIP = INADDR_NONE;
  
   // Open serial communications and wait for port to open:
    Serial.begin(115200);
  
    pinMode(GROUND_PIN,           OUTPUT);
    digitalWrite(GROUND_PIN,      LOW);
  
    // start the Ethernet connection and the server:
    if (ethernetEnabled) {
        while (localIP == INADDR_NONE) {
              Serial.println("ethernet enabled");
              Ethernet.begin(mac);
              localIP = Ethernet.localIP();
              Serial.print("server is at ");
              Serial.println(localIP);
        }
    }
    if (localIP == INADDR_NONE) {
        Serial.println("ethernet disabled");
        ethernetEnabled = 0;
    } else {
        server.begin();
    }
    sensorList.addSensor(new DHTSensor(DHT_PIN, DHTSensorType_AM2301));
    sensorList.addSensor(new AnalogSensor("Light", LIGHT_PIN, 500, 0));
    sensorList.addSensor(new AnalogSensor("Heater", HEATER_PIN, 500, 650));
    sensorList.begin();
}

void loop()
{
    sensorList.loop();
    if (ethernetEnabled) {
        EthernetClient client = server.available();
        
        if (client) {
            Serial.println("client");
            
            sensorList.printInfo(&client, NODE_ID);
            sensorList.printInfo(&Serial, NODE_ID);
            while (client.connected()) {
                char command[255];
                unsigned int index = 0;
                boolean commandDone = false;
                
                command[0] = 0;
                while (client.connected()) {
                    char a;
                    
                    if (client.available() > 0) {
                        a = client.read();
                        if (a == '\r') {
                        } else if (a == '\n') {
                            commandDone = true;
                            break;
                        } else {
                            command[index] = a;
                            index++;
                            command[index] = 0;
                        }
                    }
                    if (index == sizeof(command)) {
                        commandDone = true;
                    }
                }
                if (commandDone) {
                    if (strcmp(command, "zob") == 0) {
                        client.println("zob toi meme");
                    } else if (strcmp(command, "munin-values") == 0) {
                        Serial.println("radiateur reseted");
                        sensorList.resetValues();
                    } else if (command[0] == 0 || strcmp(command, "stop") == 0) {
                        client.flush();
                        client.stop();
                    } else {
                        Serial.print("Error: ");
                        Serial.println(command);
                    }
                    index = 0;
                    command[index] = 0;
                }
            }
        }
    } else {
        sensorList.printInfo(&Serial, NODE_ID);
        Serial.println("--");
        delay(1000);
    }
    blinker.loop();
}

