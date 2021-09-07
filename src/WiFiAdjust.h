// подключени библиотек
#include <SD.h>
int chipSelect = 4;
bool removeFlag;
File myFile;
#if defined(ESP8266)
#include <ESP8266WiFi.h> //https://github.com/esp8266/Arduino
#else
#include <WiFi.h>
#endif
#include <ESPAsyncTCP.h>
#include <Hash.h>
#include <FS.h>
#include <ESPAsyncWebServer.h>
#include <LITTLEFS.h>
#include <DNSServer.h>
// #include <WiFiManager.h>

#include <ESPAsyncWiFiManager.h> //https://github.com/tzapu/WiFiManager

AsyncWebServer server(80);
DNSServer dns;

// Create an Event Source on /events
AsyncEventSource events("/events");
// большая строка в которой будут храниться данные в формате [ИМЯ1:ЗНАЧЕНИЕ1;ИМЯ2:ЗНАЧЕНИЕ2...]
String GeneralString;
// в этот файл буду сохранять строку
const char *MyFile = {"/MyFile.txt"};
// временные файлы для работы внутри функций
String TemporaryFile1;
String TemporaryFile2;
#include "private_stuff"
// функции для работы с файлами, чтения , открытия и записи
void notFound(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Not found");
}

String readFile(String ThisFile)
{
    String fileContent;
    myFile = SD.open(ThisFile);

    // read from the file until there's nothing else in it:
    while (myFile.available())
    {
        fileContent += char(myFile.read());
    }
    myFile.close();
    return fileContent;
}

void writeFile(String FileName, String fileContent)
{
    if (SD.exists(FileName))
        SD.remove(FileName);
    myFile = SD.open(FileName, FILE_WRITE);

    // if the file is available, write to it:
    myFile.println(fileContent);
    myFile.close();
}

void writeFile(String FileName, String fileContent, bool rem)
{
    myFile = SD.open(FileName, FILE_WRITE);
    // if the file is available, write to it:
    myFile.println(fileContent);
    myFile.close();
}
void writeFile(fs::FS &fs, const char *path, String message)
{
    //File file = fs.open(path, "w");
}

void writeFile(fs::FS &fs, const char *path, const char *message)
{
    //File file = fs.open(path, "w");
}
// функция - обработчик. Она нахоит метки в html коде и меняет их на переменные
String processor2(const String &var)
{
    for (int j = 0; j < leng; j++)
    {
        if (var == parametr[j])
        {
            TemporaryFile1 = parametr[j];
            TemporaryFile2 = "";
            if (j != leng - 1)
            {
                for (int i = GeneralString.indexOf(parametr[j]) + TemporaryFile1.length(); i < GeneralString.indexOf(parametr[j + 1]); i++)
                {
                    TemporaryFile2 += GeneralString[i];
                }
            }
            else if (j == leng - 1)
            {
                for (int i = GeneralString.indexOf(parametr[j]) + TemporaryFile1.length(); i < GeneralString.length(); i++)
                {
                    TemporaryFile2 += GeneralString[i];
                }
            }
            return TemporaryFile2;
        }
    }
    return String();
}
// функция для получения значения из строки
String GettingValueFromString(int j)
{
    TemporaryFile1 = parametr[j];
    TemporaryFile2 = "";
    if (j != leng - 1)
    {
        for (int i = GeneralString.indexOf(parametr[j]) + TemporaryFile1.length(); i < GeneralString.indexOf(parametr[j + 1]); i++)
        {
            TemporaryFile2 += GeneralString[i];
        }
    }
    else if (j == leng - 1)
    {
        for (int i = GeneralString.indexOf(parametr[j]) + TemporaryFile1.length(); i < GeneralString.length(); i++)
        {
            TemporaryFile2 += GeneralString[i];
        }
    }
    return TemporaryFile2;
}

// функция для записи значения в строку
void SendingValueToString(int j, int k)
{
    TemporaryFile1 = parametr[j];
    TemporaryFile2 = "";
    if (j != leng - 1)
    {
        for (int i = GeneralString.indexOf(parametr[j]) + TemporaryFile1.length(); i < GeneralString.indexOf(parametr[j + 1]); i++)
        {
            TemporaryFile2 += GeneralString[i];
        }
    }
    else if (j == leng - 1)
    {
        for (int i = GeneralString.indexOf(parametr[j]) + TemporaryFile1.length(); i < GeneralString.length(); i++)
        {
            TemporaryFile2 += GeneralString[i];
        }
    }
    GeneralString.replace(parametr[j] + TemporaryFile2, parametr[j] + String(k));
}
void initSDCard()
{

    if (SD.exists("datalog.txt"))
    {
        GeneralString = readFile("datalog.txt");
        for (int i = 0; i < leng - 1; i++)
        {
            InternetSensorState[i] = 0;
            CurrentSensorState[i] = GettingValueFromString(i).toInt();
            PreviousSensorState[i] = CurrentSensorState[i];
            if (CurrentSensorState[i] == 254)
            {
                removeFlag = 1;
                Serial.println();
                Serial.print("Wrong number is  ");
                Serial.println(i);
            }
            Serial.print(i);
            Serial.print("/");
            Serial.print(CurrentSensorState[i]);
            Serial.print(" ");
        }
        Serial.println("File was exists");
        if (removeFlag)
        {
            SD.remove("datalog.txt");
            Serial.println("date was wrong, I remove file");
        }
    }
    // if file dont exist
    if (!SD.exists("datalog.txt"))
    {
        GeneralString = "";
        for (int i = 0; i < leng - 1; i++)
        {
            GeneralString += parametr[i] + String(0);
        }
        GeneralString += parametr[leng - 1] + String(254);
        Serial.println("file was NOT exists and I created fule from zero");
        writeFile("datalog.txt", GeneralString);
        if (SD.exists("datalog.txt"))
            Serial.println("It become exist in the end");
        else
            Serial.println("I couldn't create file");
    }
}
void WiFiSetup()
{
    Serial.println("ESP8266 INIT");
    Serial.println(F("Inizializing FS..."));
    (LittleFS.begin()) ? Serial.println(F("done.")) : Serial.println(F("fail."));
    SD.begin(D8);
    Serial.println("I going to init SD");
    //SD.remove("datalog.txt");

    initSDCard();
    Serial.println("I going to gheck data");
    // writeFile(LittleFS, MyFile, GeneralString);

    // получение сораненных значений
    for (int i = 0; i < leng; i++)
    {
        InternetSensorState[i] = 0;
        CurrentSensorState[i] = GettingValueFromString(i).toInt();
        PreviousSensorState[i] = CurrentSensorState[i];
    }
    SendingValueToString(leng - 1, 254);
    Serial.println(GeneralString);
    writeFile("datalog.txt", GeneralString);
    // намеренно меняю первые данные, чтоб потом получит верный отзыв от Меги
    CurrentSensorState[0] = 66;
    CurrentSensorState[1] = 66;
    //  SendingValueToString(0, CurrentSensorState[0]);
    //  SendingValueToString(1, CurrentSensorState[1]);
    // настройка wifi
    AsyncWiFiManager wifiManager(&server, &dns);
    wifiManager.autoConnect("AutoConnectAP");
    // доступ к странице
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/index.html", "text/html", 0, processor2); });

    server.serveStatic("/", LittleFS, "/");
    /* слушаем события на странцие, если событие произошло, 
    записываем его в буфер и поднимаем флаг InternetFlag
    */

    server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                  String inputMessage;
                  Serial.println("Get date from internet stuff");
                  for (int i = 46; i < leng - 1; i++)
                  {
                      if (request->hasParam(parametr[i]) && (i < 170 || i == 190 || i == 192)) // changed
                      {
                          // обработка часов
                          inputMessage = request->getParam(parametr[i])->value();
                          InternetSensorState[i] = inputMessage.substring(0, 2).toInt();
                          SendingValueToString(i, InternetSensorState[i]);
                          //Serial.print("InternetSensorState[i]  ");
                          //Serial.println(InternetSensorState[i]);
                          inter[i] = i;
                          i++;
                          InternetSensorState[i] = inputMessage.substring(3, 5).toInt();
                          SendingValueToString(i, InternetSensorState[i]);
                          //Serial.print("InternetSensorState[i]  ");
                          //Serial.println(InternetSensorState[i]);
                          inter[i] = i;
                      }
                      else if (request->hasParam(parametr[i]) && (i == 170 || i == 180)) // changed
                      {
                          // обработка даты дд.мм.гггг.чч.мм
                          inputMessage = request->getParam(parametr[i])->value();
                          if (inputMessage.length() > 16)
                          {
                              inputMessage = inputMessage.substring(inputMessage.length() - 16, 18);
                          }
                          /*Serial.print("parametr[i]  ");
                          Serial.print(parametr[i]);
                          Serial.print("  inputMessage[i]  ");
                          Serial.println(InternetSensorState[i]);
                            */
                          inter[i] = i;
                          InternetSensorState[i] = inputMessage.substring(5, 7).toInt();
                          SendingValueToString(i, InternetSensorState[i]);
                          i++;
                          inter[i] = i;
                          /*Serial.print("parametr[i]  ");
                          Serial.print(parametr[i]);
                          Serial.print("  inputMessage[i]  ");
                          Serial.println(InternetSensorState[i]);
                            */
                          InternetSensorState[i] = inputMessage.substring(8, 11).toInt();
                          SendingValueToString(i, InternetSensorState[i]);
                          i++;
                          inter[i] = i;
                          /*Serial.print("parametr[i]  ");
                          Serial.print(parametr[i]);
                          Serial.print("  inputMessage[i]  ");
                          Serial.println(InternetSensorState[i]);
                            */
                          InternetSensorState[i] = inputMessage.substring(11, 14).toInt();
                          SendingValueToString(i, InternetSensorState[i]);
                          i++;
                          inter[i] = i;
                          InternetSensorState[i] = inputMessage.substring(14, 17).toInt();
                          SendingValueToString(i, InternetSensorState[i]);
                          /*Serial.print("parametr[i]  ");
                          Serial.print(parametr[i]);
                          Serial.print("  inputMessage[i]  ");
                          Serial.println(InternetSensorState[i]);
                            */
                      }
                      else if (request->hasParam(parametr[i]) && i > 170 && i != 180 && i != 174 && i != 184) // changed
                      {
                          inputMessage = request->getParam(parametr[i])->value();
                          InternetSensorState[i] = inputMessage.toInt();
                          SendingValueToString(i, InternetSensorState[i]);
                          Serial.println(InternetSensorState[i]);
                          inter[i] = i;
                      }
                      else if (request->hasParam(parametr[i]) && (i == 174 || i == 184)) // changed
                      {
                          inputMessage = request->getParam(parametr[i])->value();
                          InternetSensorState[i + 5] = inputMessage.toInt() % 24;
                          InternetSensorState[i + 4] = (inputMessage.toInt() - InternetSensorState[i + 5]) / 24;
                          inter[i + 5] = i + 5;
                          inter[i + 4] = i + 4;
                          /*
                          Serial.println("Cycle change");
                          Serial.print(parametr[i + 4]);
                          Serial.print("\t");
                          Serial.println(InternetSensorState[i + 4]);
                          Serial.print(parametr[i + 5]);
                          Serial.print("\t");
                          Serial.println(InternetSensorState[i + 5]);
                          */
                          SendingValueToString(i + 4, InternetSensorState[i + 4]);
                          SendingValueToString(i + 5, InternetSensorState[i + 5]);
                          SendingValueToString(i, InternetSensorState[i + 4] * 24 + InternetSensorState[i + 5]);
                          events.send(String(InternetSensorState[i + 4] * 24 + InternetSensorState[i + 5]).c_str(), parametr[i], millis());
                      }
                      if (!Internet_flag)
                          Internet_flag = 1;
                  }
                  Serial.println("I finished get date from internet stuff");
                  request->send(200, "text/text", inputMessage);
              });
    server.onNotFound(notFound);

    events.onConnect([](AsyncEventSourceClient *client)
                     {
                         if (client->lastId())
                         {
                             Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
                         }
                         client->send("hello!", NULL, millis(), 10000);
                     });
    server.addHandler(&events);
    // Start server
    server.begin();

// other adjust
#ifndef ESP8266
    while (!Serial)
        ; // wait for serial port to connect. Needed for native USB
#endif
}
