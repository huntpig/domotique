/*
  Web Server
 
  This sketch acts as a server using a RedFly-Shield. 
 */

#include "Arduino.h"
#include "ElectricMeter.h"
#include "TemperatureDallas.h"
#include "DHT.h"
#include "Adafruit_BMP085.h"

#define COMPTEUR1           5
#define COMPTEUR2           6
#define DHT_PIN             2

typedef struct _DallasList {
    TemperatureDallas *dallas;
    struct _DallasList *next;
} DallasList;

ElectricMeter electricMeter;
DallasList *dallasList = NULL;
OneWire  ds(8);  // on pin 10
DHT dht(DHT_PIN, DHT22);
Adafruit_BMP085 bmp;

static int find_address(OneWire *ds, uint8_t *address)
{
    while (1) {
        if (!ds->search(address)) {
            ds->reset_search();
            return 0;
        }
        if (OneWire::crc8(address, 7) == address[7]) {
            return 1;
        }
    }
}

void setup()
{
    uint8_t address[8];
    
    pinMode(3, OUTPUT);
    pinMode(4, OUTPUT);
    digitalWrite(3, LOW);
    digitalWrite(4, HIGH);
    
    electricMeter.addMeterPin(COMPTEUR1);
    electricMeter.addMeterPin(COMPTEUR2);
    electricMeter.enableMeter(COMPTEUR1);
    electricMeter.setSerialPort(&Serial1);
    
    while (find_address(&ds, address)) {
        DallasList *element = (DallasList *)malloc(sizeof(DallasList));
        
        element->dallas = new TemperatureDallas(address, &ds);
        element->next = dallasList;
        dallasList = element;
    }
    bmp.begin();
    Serial.begin(115200);
    Serial.println("ok");
}

void test_loop()
{
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;
  
  if ( !ds.search(addr)) {
    Serial.println("No more addresses.");
    Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }
  
  Serial.print("ROM =");
  for( i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
  Serial.println();
 
  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      Serial.println("  Chip = DS18S20");  // or old DS1820
      type_s = 1;
      break;
    case 0x28:
      Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      Serial.println("Device is not a DS18x20 family device.");
      return;
  } 

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1);         // start conversion, with parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  Serial.print("  Data = ");
  Serial.print(present,HEX);
  Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.print(" CRC=");
  Serial.print(OneWire::crc8(data, 8), HEX);
  Serial.println();

  // convert the data to actual temperature

  unsigned int raw = (data[1] << 8) | data[0];
  if (type_s) {
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
  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
  Serial.print("  Temperature = ");
  Serial.print(celsius);
  Serial.print(" Celsius, ");
  Serial.print(fahrenheit);
  Serial.println(" Fahrenheit");
  
  char buffer[25];

    dtostrf(15.4, 1, 2, buffer);
    Serial.print(buffer);
}

void loop()
{
    uint8_t sock;
    char buf[512], *ptr;
    uint16_t buf_len, rd, len;
    DallasList *cursor;
    
    cursor = dallasList;
    while (cursor) {
        cursor->dallas->loop();
        cursor = cursor->next;
    }
    //check if socket is opened
    if (electricMeter.listenPort()) {
        char buffer[32];
        int ii = 1;
        float dhtTemperature = dht.readTemperature();
        float dhtHumidity = dht.readHumidity();
        float bmpTemperature = bmp.readTemperature();
        float bmpPressure = bmp.readPressure();
        
        Serial.print(electricMeter.data());
        cursor = dallasList;
        while (cursor) {
            Serial.print("DALLAS_TEMP");
            Serial.print(ii);
            Serial.print(" ");
            dtostrf(cursor->dallas->celsius(), 1, 2, buffer);
            Serial.println(buffer);
            
            Serial.print("DALLAS_TEMP_TYPE");
            Serial.print(ii);
            Serial.print(" ");
            Serial.println(cursor->dallas->typeName());
            cursor = cursor->next;
            ii++;
        }
        Serial.print("DHT_HUMIDITY ");
        Serial.println(dhtHumidity);
        Serial.print("DHT_TEMP ");
        Serial.println(dhtTemperature);
        Serial.print("BMP_PRESSURE ");
        Serial.println(bmpPressure);
        Serial.print("BMP_TEMP ");
        Serial.println(bmpTemperature);
        Serial.println("--");
    }
}