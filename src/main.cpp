#include <Arduino.h>
#include <Wire.h>

// если произошло событие на сайте  Internet_flag будет равен 1
int Internet_flag = 0;
// адрес меги
#define SLAVE_ADDR 7
// Если на этом пине 1, меха хочет передать данные
const int SlavePin = A0; // d7
// флаг для приема данных
int CurrentGetFlag = 0;
int PreviousGetFlag = 0;

// количество хранимых переменных
const int leng = 199;
// текущее значение переменных
int CurrentSensorState[leng];
// предыдущее значение
int PreviousSensorState[leng];
// буфер для i2c передачи
int buffer[leng];
// буфер для получения данных с интернета
int InternetSensorState[leng];
int inter[leng];
// флаг сигнализирующий о первой передаче
bool FirstTimeFlag = 1;

unsigned long TimeOfAsking;
// сщетчик байтов и байт для i2c передачи
int bcount;
byte bval;
// настройка работы с сервером
#include <WiFiAdjust.h>
// переменная для хранения IP адреса
String adr;

// функция получения данных от меги
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

// функция отправки данных в Мегу первый раз

void FirstTimeSendData()
{
  Serial.println("Send data first time");
  adr = WiFi.localIP().toString();
  //передача IP адреса при включении
  for (int i = 0; i < adr.length(); i++)
  {
    Wire.beginTransmission(SLAVE_ADDR);
    bval = adr[i];
    Wire.write(bval);
    Wire.endTransmission();
    Serial.print(adr[i]);
  }
  Serial.println();
  Wire.beginTransmission(SLAVE_ADDR);
  CurrentSensorState[leng - 1] = 254;
  Wire.write(CurrentSensorState[leng - 1]);
  Wire.endTransmission();
  // Адррес передали
  // передачаненулевых данных
  byte bval;
  bval = 255; // send key-start
  Wire.beginTransmission(SLAVE_ADDR);
  Wire.write(bval);
  Wire.endTransmission();
  delay(10);
  for (int i = 0; i < leng - 2; i++)
  {
    if (CurrentSensorState[i] != 0)
    {
      bval = i;
      Wire.beginTransmission(SLAVE_ADDR);
      Wire.write(bval);
      Wire.endTransmission();
      bval = CurrentSensorState[i];
      Wire.beginTransmission(SLAVE_ADDR);
      Wire.write(bval);
      Wire.endTransmission();
      Serial.print(i);
      Serial.print("/");
      Serial.print(CurrentSensorState[i]);
      Serial.print("  ");
      delay(10);
    }
  }
  Serial.println();
  Wire.beginTransmission(SLAVE_ADDR);
  CurrentSensorState[leng - 1] = 254;
  Wire.write(CurrentSensorState[leng - 1]);
  Wire.endTransmission();
  Serial.println("I sendED data first time");
}
void setup()
{
  Serial.begin(115200);
  /*for (int i = 0; i < leng; i++)
  {
    Serial.print("parametr[");
    Serial.print(i);
    Serial.print("]  ");
    Serial.println(parametr[i]);
  }
  */
  //установка пина для того чтобы слушать мегу
  WiFiSetup();
  /* задаем i2c мост через контакты SDA=D1 и SCL=D2 на NodeMCU */
  Wire.begin(D1, D2);
  // записываем IP адрес
  Serial.println("wait before send first data");
  Serial.println("fun begin");
  FirstTimeSendData();
}
int k;
unsigned long tiki = 0;
bool sdlog;
void loop()
{
  // CurrentGetFlag = 1;
  if ((millis() - tiki) > 200)
  {
    tiki = millis();
    PreviousGetFlag = CurrentGetFlag;
    CurrentGetFlag = analogRead(A0);
    if (CurrentGetFlag > 600)
    {
      Serial.println("HIGH");
    }
    //else
    // Serial.print("I here");
  }
  /*
  // слушаем мегу
  // если мега хочет нам что то сказать, при этом в сети тихо, то слушаем мегу
 */
  //
  //CurrentGetFlag = 1;

  if (CurrentGetFlag > 600 && (PreviousGetFlag < 600) && !Internet_flag)
  {
    // начинаем принимать данные
    Serial.println("I get date from MEGA");

    while (readI2C(SLAVE_ADDR) < 255)
    {
      // Until first byte has been received print a waiting message
      Serial.print("Waiting");
    }
    bcount = 0;
    // принимать данные пока не наталкнемся на ключ
    while (buffer[bcount - 1] != 254)
    {
      // записываем данные в буфер
      buffer[bcount] = readI2C(SLAVE_ADDR);
      bcount++;
    }
    // обновляем текущие данные
    if (buffer[0] != 255)
    {
      for (int i = 0; i < bcount - 1; i = i + 2)
      {
        CurrentSensorState[buffer[i]] = buffer[i + 1];
        Serial.print(buffer[i]);
        Serial.print("/");
        Serial.print(CurrentSensorState[buffer[i]]);
        Serial.print(" ");
      }
    }
    else
    {
      for (int i = 1; i < bcount - 1; i = i + 2)
      {
        CurrentSensorState[buffer[i]] = buffer[i + 1];
        Serial.print(buffer[i]);
        Serial.print("/");
        Serial.print(CurrentSensorState[buffer[i]]);
        Serial.print(" ");
      }
    }
    Serial.println(" ");
    for (int i = 0; i < leng; i++)
    {
      //переопределяем и публикуем только новые данные
      if (CurrentSensorState[i] != PreviousSensorState[i] && i != 174 && i != 184 && i != 178 && i != 179 && i != 188 && i != 189)
      {
        PreviousSensorState[i] = CurrentSensorState[i];
        SendingValueToString(i, CurrentSensorState[i]);
        events.send(String(CurrentSensorState[i]).c_str(), parametr[i], millis());
        if (i != 1)
          sdlog = 1;
      }
      if (CurrentSensorState[i] != PreviousSensorState[i] && (i == 178 || i == 188))
      {
        PreviousSensorState[i] = CurrentSensorState[i];
        SendingValueToString(i, CurrentSensorState[i]);
        CurrentSensorState[i - 4] = CurrentSensorState[i] * 24;
        events.send(String(CurrentSensorState[i] * 24).c_str(), parametr[i - 4], millis());
      }
      else if (CurrentSensorState[i] != PreviousSensorState[i] && (i == 179 || i == 189))
      {
        PreviousSensorState[i] = CurrentSensorState[i];
        SendingValueToString(i, CurrentSensorState[i]);
        CurrentSensorState[i - 5] += CurrentSensorState[i];
        events.send(String(CurrentSensorState[i - 1] * 24 + CurrentSensorState[i]).c_str(), parametr[i - 5], millis());
      }
    }
    // сохраняем все
    //writeFile(LittleFS, MyFile, GeneralString);
    writeFile("datalog.txt", GeneralString);
    /*
    if (CurrentSensorState[38])
      writeFile("AllDataLog.txt", GeneralString);
    if (sdlog)
    {
      writeFile("AllDataLog.txt", GeneralString, 1);
      sdlog = 0;
    }
    */
    // сбрасываю флаг
    Serial.println("The date was recived");
    PreviousGetFlag = CurrentGetFlag;
  }

  // Если в интернете что то произошло
  if (Internet_flag)
  {
    Serial.println("Something has happend in internet");
    // сохраняем данные
    //writeFile(LittleFS, MyFile, GeneralString);
    writeFile("datalog.txt", GeneralString);
    // отправляем данные меге
    byte bval;
    bval = 255; // send key-start
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write(bval);
    Wire.endTransmission();
    for (int i = 14; i < leng; i++)
    {
      if (InternetSensorState[i] != CurrentSensorState[i] && inter[i] != 0 && i != 174 && i != 184)
      {
        Serial.print(i);
        Serial.print("_");
        Serial.print(InternetSensorState[i]);
        Serial.print(" ");
        CurrentSensorState[i] = InternetSensorState[i];
        bval = i;
        Wire.beginTransmission(SLAVE_ADDR);
        Wire.write(bval);
        Wire.endTransmission();
        bval = CurrentSensorState[i];
        Wire.beginTransmission(SLAVE_ADDR);
        Wire.write(bval);
        Wire.endTransmission();
        inter[i] = 0;
      }
    }
    Wire.beginTransmission(SLAVE_ADDR);
    CurrentSensorState[leng - 1] = 254;
    Wire.write(CurrentSensorState[leng - 1]);
    Wire.endTransmission();

    // обнуляем флаги
    Internet_flag = 0;
    FirstTimeFlag = 0;
    Serial.println("I sended date from Inet");
  }
}
