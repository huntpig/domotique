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
#include "DHT.h"
#include "Blinker.h"

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0D, 0x5C, 0xDD };

// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
#define SERVER_PORT      10000
#define DHT_PIN          8
#define LED_PIN          9
#define LIGHT_PIN        A0
#define RADIATEUR_PIN    A1
#define GROUND_PIN       A2
#define ETHERNET_ENABLED 1

DHT dht(DHT_PIN, DHT22);
Blinker blinker(LED_PIN);
EthernetServer server(SERVER_PORT);
char ethernetEnabled = ETHERNET_ENABLED;

void setup()
{
  IPAddress localIP = INADDR_NONE;

 // Open serial communications and wait for port to open:
  Serial.begin(115200);

  pinMode(LIGHT_PIN,            INPUT);
  pinMode(RADIATEUR_PIN,        INPUT);
  pinMode(GROUND_PIN,           OUTPUT);
  digitalWrite(LIGHT_PIN,       HIGH);
  digitalWrite(RADIATEUR_PIN,   HIGH);
  digitalWrite(GROUND_PIN,     LOW);

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
}

int radiateur_count = 0;
int radiateur_value = 0;
unsigned long lastQueryTime = 0;

void writeToStream(Stream *client)
{
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    
    if (isnan(t) || isnan(h)) {
        client->println("error");
    } else {
        client->print("H "); 
        client->println(h);
        client->print("T "); 
        client->println(t);
    }
    client->print("L ");
    client->println(analogRead(LIGHT_PIN));
    client->print("IR ");
    client->println(analogRead(RADIATEUR_PIN));
    client->print("AR ");
    client->println(((float)radiateur_value) / ((float)radiateur_count));
    client->print("ARCount ");
    client->println(radiateur_count);
    client->print("ARValue ");
    client->println(radiateur_value);
}

void loop()
{
    unsigned long currentTime = millis();
    if (currentTime < lastQueryTime || currentTime > lastQueryTime + 500) {
        lastQueryTime = currentTime;
        radiateur_count++;
        if (analogRead(RADIATEUR_PIN) < 650) {
            radiateur_value++;
        }
    }
    if (ethernetEnabled) {
        EthernetClient client = server.available();
        
        if (client) {
            if (client.available() >= 8) {
            char buffer[32];
            int count;
            
            count = client.read((uint8_t *)buffer, sizeof(buffer) - 1);
            buffer[count] = 0;
            if (strcmp(buffer, "values\r\n") == 0) {
                writeToStream(&client);
                writeToStream(&Serial);
            } else if (strcmp(buffer, "reset_radiateur\r\n") == 0) {
                client.println("radiateur reseted");
                Serial.println("radiateur reseted");
                radiateur_value = 0;
                radiateur_count = 0;
            } else {
                client.flush();
            }
        }
        client.stop();
        }
    } else {
        writeToStream(&Serial);
        delay(500);
    }
    blinker.blink();
}

