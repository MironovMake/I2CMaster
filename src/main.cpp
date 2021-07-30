
#include <Arduino.h>
#include <SPI.h>
#include <OneWire.h>
#include <Wire.h>
unsigned long prev;
int Internet_flag = 0;
int change;
int m;
// Master Adjust
#define SLAVE_ADDR 11
bool MasterFlag = 0;
uint8_t SlavePin = 12; // its D6
int bcount;
const int leng = 188;
int CurrentSensorState[leng];
int PreviousSensorState[leng];
int buffer[leng];
int InternetSensorState[leng];
unsigned long OutOffHere;
bool Mega_listen = 0;
int n;
bool FirstTimeFlag = 1;
byte bval;
unsigned long LastTimeGeting;
// WIFI ADJUSTs
#include <WiFiAdjust.h>
String adr;

byte readI2C(int address)
{
  // Define a variable to hold byte of data
  byte bval;
  long entry = millis();
  // Read one byte at a time
  Wire.requestFrom(address, 1);
  // Wait 100 ms for data to stabilize
  while (Wire.available() == 0 && (millis() - entry) < 100)
    ;
  ;
  // Place data into byte
  if (millis() - entry < 100)
    bval = Wire.read();
  return bval;
}
void FirstTimeSendData()
{
  Serial.println("I send date first time");
  byte bval;
  bval = 255; // send key-start
  Wire.beginTransmission(SLAVE_ADDR);
  Wire.write(bval);
  Wire.endTransmission();
  for (int i = 0; i < leng-2; i++)
  {
    if (CurrentSensorState[i] != 0)
    {
      bval = i;
      Wire.beginTransmission(SLAVE_ADDR);
      Wire.write(bval);
      Wire.endTransmission();
      Serial.print(bval);
      Serial.print(" ");
      Serial.print(CurrentSensorState[i]);
      Serial.print(" ");
      bval = CurrentSensorState[i];
      Wire.beginTransmission(SLAVE_ADDR);
      Wire.write(bval);
      Wire.endTransmission();
    }
  }
  Wire.beginTransmission(SLAVE_ADDR);
  CurrentSensorState[leng - 1] = 254;
  Wire.write(CurrentSensorState[leng - 1]);
  Wire.endTransmission();
  Serial.println();
  Serial.println("I sended data to mega first time");
}

void setup()
{
  Serial.begin(115200);
  Serial.println("ESP8266 INIT");
  Serial.println(F("Inizializing FS..."));
  pinMode(SlavePin, INPUT);
  // Initialize LittleFS
  WiFiSetup();
  Wire.begin(D1, D2); /* задаем i2c мост через контакты SDA=D1 и SCL=D2 на NodeMCU */

  Serial.println(WiFi.localIP());
  Serial.println(WiFi.localIP());

  adr = WiFi.localIP().toString();
  //передача IP адреса при включении
  for (int i = 0; i < adr.length(); i++)
  {
    Wire.beginTransmission(SLAVE_ADDR);
    bval = adr[i];
    Wire.write(bval);
    Wire.endTransmission();
  }
  Wire.beginTransmission(SLAVE_ADDR);
  CurrentSensorState[leng - 1] = 254;
  Wire.write(CurrentSensorState[leng - 1]);
  Wire.endTransmission();

  FirstTimeSendData();
}

void loop()
{
  if (digitalRead(SlavePin) && !Internet_flag)
  {
    Serial.println("I am getting date from Mega");
    // get date

    while (readI2C(SLAVE_ADDR) < 255)
    {
      // Until first byte has been received print a waiting message
      Serial.print("Waiting");
    }
    bcount = 0;
    while (buffer[bcount - 1] != 254)
    {
      buffer[bcount] = readI2C(SLAVE_ADDR);
      Serial.print(buffer[bcount]);
      Serial.print(" ");
      bcount++;
    }

    Serial.println();
    for (int i = 0; i < bcount - 1; i = i + 2)
    {
      CurrentSensorState[buffer[i]] = buffer[i + 1];
      Serial.print(CurrentSensorState[buffer[i]]);
      Serial.print(" ");
    }
    Serial.println();

    // save date to files, than post ones in internet
    for (int i = 0; i < leng; i++)
    {
      //Serial.print(" ");
      // Serial.print(CurrentSensorState[i]);
      if (CurrentSensorState[i] != PreviousSensorState[i] && i != 179 && i != 180)
      {
        PreviousSensorState[i] = CurrentSensorState[i];
        SendingValueToString(i, CurrentSensorState[i]);
        events.send(String(CurrentSensorState[i]).c_str(), parametr[i], millis());
      }
      if (CurrentSensorState[i] != PreviousSensorState[i] && (i == 179 || i == 180))
      {
        PreviousSensorState[i] = CurrentSensorState[i];
        SendingValueToString(i, CurrentSensorState[i]);
        events.send(String(CurrentSensorState[179] * 24 + CurrentSensorState[180]).c_str(), parametr[177], millis());
      }
    }

    writeFile(LittleFS, MyFile, GeneralString);
    Serial.println("I success get date from slave");
    /*if (CurrentSensorState[0] > 23)
    {
      InternetSensorState[0] = 0;
      Internet_flag = 1;
    }
    if (CurrentSensorState[1] > 59)
    {
      InternetSensorState[1] = 0;
      Internet_flag = 1;
    }*/
  }
  
  if (Internet_flag)
  {
    Serial.println();
    Serial.println("Something happening in internet");
    writeFile(LittleFS, MyFile, GeneralString);

    byte bval;
    bval = 255; // send key-start
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write(bval);
    Wire.endTransmission();
    for (int i = 0; i < leng; i++)
    {
      if (InternetSensorState[i] != CurrentSensorState[i])
      {
        CurrentSensorState[i] = InternetSensorState[i];
        Serial.println(i);
        Serial.println("/");
        Serial.println(CurrentSensorState[i]);
        Serial.println(" ");
        bval = i;
        Wire.beginTransmission(SLAVE_ADDR);
        Wire.write(bval);
        Wire.endTransmission();
        Serial.print(CurrentSensorState[i]);
        Serial.print(" ");
        bval = CurrentSensorState[i];
        Wire.beginTransmission(SLAVE_ADDR);
        Wire.write(bval);
        Wire.endTransmission();
      }
    }
    Wire.beginTransmission(SLAVE_ADDR);
    CurrentSensorState[leng - 1] = 254;
    Wire.write(CurrentSensorState[leng - 1]);
    Wire.endTransmission();

    // Serial.println();
    Serial.println("I sended data to mega");
    Internet_flag = 0;
    FirstTimeFlag = 0;
    delay(500);
  }
  //Serial.println("eher");
}