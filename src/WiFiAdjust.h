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

// называю переменные
const char *parametr[leng] = {
    "HOUR", "MIN", "TEMP", "LEAK", "LIGHT",
    "BOBBER1", "BOBBER2", "BOBBER3", "BOBBER4",
    "SWITCH1", "SWITCH2", "SWITCH3", "SWITCH4",
    "OBTACLE1", "OBTACLE2",
    "STEP1_STATE", "STEP2_STATE", "STEP3_STATE", "STEP4_STATE",
    "SOLI", "LASER1", "LASER2",
    "VALVE1", "VALVE2", "VALVE3", "VALVE4", "VALVE5", "VALVE6",
    "PUMP1", "PUMP2", "PUMP3", "PUMP4", "BIG_PUMP",
    "DEVICE1", "DEVICE2", "DEVICE3", "DEVICE4", "DEVICE5", "DEVICE6", "DEVICE7", "DEVICE8",
    "DRIVE1", "DRIVE2", "DRIVE3", "DRIVE4", "LED",
    "STEP3_hour_on", "STEP3_min_on", "STEP3_hour_off", "STEP3_min_off",
    "STEP4_hour_on", "STEP4_min_on", "STEP4_hour_off", "STEP4_min_off",
    "SOLI_hour_on", "SOLI_min_on", "SOLI_hour_off", "SOLI_min_off",
    "LASER1_hour_on", "LASER1_min_on", "LASER1_hour_off", "LASER1_min_off",
    "LASER2_hour_on", "LASER2_min_on", "LASER2_hour_off", "LASER2_min_off",
    "VALVE1_hour_on", "VALVE1_min_on", "VALVE1_hour_off", "VALVE1_min_off",
    "VALVE2_hour_on", "VALVE2_min_on", "VALVE2_hour_off", "VALVE2_min_off",
    "VALVE3_hour_on", "VALVE3_min_on", "VALVE3_hour_off", "VALVE3_min_off",
    "VALVE4_hour_on", "VALVE4_min_on", "VALVE4_hour_off", "VALVE4_min_off",
    "VALVE5_hour_on", "VALVE5_min_on", "VALVE5_hour_off", "VALVE5_min_off",
    "VALVE6_hour_on", "VALVE6_min_on", "VALVE6_hour_off", "VALVE6_min_off",
    "PUMP1_hour_on", "PUMP1_min_on", "PUMP1_hour_off", "PUMP1_min_off",
    "PUMP2_hour_on", "PUMP2_min_on", "PUMP2_hour_off", "PUMP2_min_off",
    "PUMP3_hour_on", "PUMP3_min_on", "PUMP3_hour_off", "PUMP3_min_off",
    "PUMP4_hour_on", "PUMP4_min_on", "PUMP4_hour_off", "PUMP4_min_off",
    "BIG_PUMP_hour_on", "BIG_PUMP_min_on", "BIG_PUMP_hour_off", "BIG_PUMP_min_off",
    "DEVICE1_hour_on", "DEVICE1_min_on", "DEVICE1_hour_off", "DEVICE1_min_off",
    "DEVICE2_hour_on", "DEVICE2_min_on", "DEVICE2_hour_off", "DEVICE2_min_off",
    "DEVICE3_hour_on", "DEVICE3_min_on", "DEVICE3_hour_off", "DEVICE3_min_off",
    "DEVICE4_hour_on", "DEVICE4_min_on", "DEVICE4_hour_off", "DEVICE4_min_off",
    "DEVICE5_hour_on", "DEVICE5_min_on", "DEVICE5_hour_off", "DEVICE5_min_off",
    "DEVICE6_hour_on", "DEVICE6_min_on", "DEVICE6_hour_off", "DEVICE6_min_off",
    "DEVICE7_hour_on", "DEVICE7_min_on", "DEVICE7_hour_off", "DEVICE7_min_off",
    "DEVICE8_hour_on", "DEVICE8_min_on", "DEVICE8_hour_off", "DEVICE8_min_off",
    "LED_hour_on", "LED_min_on", "LED_hour_off", "LED_min_off",
    "DRIVE1_hour_on", "DRIVE1_min_on", "DRIVE1_hour_off", "DRIVE1_min_off",
    "DRIVE2_hour_on", "DRIVE2_min_on", "DRIVE2_hour_off", "DRIVE2_min_off",
    "DRIVE3_hour_on", "DRIVE3_min_on", "DRIVE3_hour_off", "DRIVE3_min_off",
    "DRIVE4_hour_on", "DRIVE4_min_on", "DRIVE4_hour_off", "DRIVE4_min_off",
    "STEP1_hour_on", "STEP1_min_on", "STEP1_hour_off", "STEP1_min_off",
    "STEP2_hour_on", "STEP2_min_on", "STEP2_hour_off", "STEP2_min_off",

    "StartMonthIrrigation", "StartDayIrrigation", "StartHourIrrigation", "StartMinIrrigation",
    "IrrigationCycle", "IrrigationMix", "IrrigationDuration",
    "TankFlagIrrigation", "CycleDayIrrigation", "CycleHourIrrigation",

    "StartMonthWaterring", "StartDayWaterring", "StartHourWaterring", "StartMinWaterring",
    "WateringCycle", "WateringMix", "WateringDuration",
    "TankFlagWatering", "CycleDayWatering", "CycleHourWatering",

    "led_pattern_hour_On", "led_pattern_min_On", "led_pattern_hour_Off", "led_pattern_min_Off",

    "STEP1_value", "STEP2_value", "STEP3_value", "STEP4_value", "key"};

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
