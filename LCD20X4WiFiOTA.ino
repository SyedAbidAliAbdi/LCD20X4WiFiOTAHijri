
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <ESP8266HTTPClient.h>
#include <EEPROM.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include "NTP_Functions.h"
#include "WeatherFunctions.h"
#include "ScrollText.h"

const char* apiKey = "Your API Key";
const char* city = "Karachi";

// WiFi credentials
const char* ssid = "Your SSID";
const char* password = "Your Wifi Password";

// NTP settings
const char* ntpServer = "pk.pool.ntp.org";
const long gmtOffset_sec = 4 * 3600; // Pakistan time offset (5 hours ahead of UTC)
const int daylightOffset_sec = 0;     // No daylight saving time

// Objects and variables for NTP and server
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, gmtOffset_sec, daylightOffset_sec);
AsyncWebServer server(80);

// LCD setup
LiquidCrystal_I2C lcd(0x3F, 20, 4);


// EEPROM value and global date variable
int eepromValue = 0;
char globalDateVariable[11]; // Use char array instead of String for date

// Structure for Hijri date
struct HijriDate {
  int day;
  int month;
  int year;
  char date[20];
  char weekdayEn[20];
  char weekdayAr[20];
  char monthEn[20];
  char monthAr[20];
  char designationAbbreviated[20];
};


// NTP refresh and API call intervals
const unsigned long NTP_REFRESH_INTERVAL = 60000; // Refresh NTP time every 60 seconds
const unsigned long API_CALL_INTERVAL = 3600000;  // Call API every 60 minutes
unsigned long lastNtpRefreshTime = 0;
unsigned long lastApiCallTime = 0;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  lcd.init();
  lcd.backlight();

  // Wait for WiFi connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connecting WiFi...");
    lcd.setCursor(0, 1);
    lcd.print(ssid);
  }

  // Initialize EEPROM
  EEPROM.begin(sizeof(int));

  // Read EEPROM value
  EEPROM.get(0, eepromValue);

  // Setup server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/update", HTTP_POST, handleUpdate);

  // Start server and OTA
  AsyncElegantOTA.begin(&server);
  server.begin();
  Serial.println("HTTP server started");

  // Start NTP client
  timeClient.begin();

  // Initial screen setup
  StartScreen();
  setupNTP(ntpUDP, timeClient);

  // Gather initial data
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Gathering data...");

  updateDateTime();

  delay(1000);

  Serial.println(globalDateVariable);
  convertGregorianToHijri(globalDateVariable, eepromValue);
}

void loop() {
  unsigned long currentTime = millis();

  // Refresh NTP time every 60 seconds
  if (currentTime - lastNtpRefreshTime >= NTP_REFRESH_INTERVAL) {

    lastNtpRefreshTime = currentTime;
    updateDateTime();
    WeatherData weatherData = getWeatherData(apiKey);
    scrollText(lcd, "Scrolling Text Example generate from ChatGPT by Syed Abid Ali", true); // Scroll right to left
    lcd.setCursor(0, 2);
    lcd.print(weatherData.temp);
    Serial.println(weatherData.temp);
    Serial.println("Temprature");
  }

  // Call API every 60 minutes
  if (currentTime - lastApiCallTime >= API_CALL_INTERVAL) {

    lastApiCallTime = currentTime;
    HijriDate hijriDate = convertGregorianToHijri(globalDateVariable, eepromValue);
   
    //WeatherData weatherData = getWeatherData(apiKey);
  }
}

void handleRoot(AsyncWebServerRequest *request) {
  request->send(200, "text/html", "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'><title>Date, Time and Weather Station </title><style>body {font-family: Arial, sans-serif;margin: 0;padding: 0;background-color: #f0f0f0;}.container {max-width: 800px;margin: 0 auto;padding: 20px;text-align: center;}.card {background-color: #ffffff;border-radius: 10px;box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);padding: 20px;margin-top: 20px;}.heading {font-size: 36px;font-weight: bold;color: #333333;margin-bottom: 10px;}.subheading {font-size: 18px;color: #666666;margin-bottom: 30px;}label {display: block;margin-bottom: 10px;}input[type='number'] {width: 50px;padding: 10px;border: 1px solid #cccccc;border-radius: 5px;margin-bottom: 20px;border-radius: 5px; /* Round the corners */}input[type='submit'] {background-color: #4CAF50;color: white;padding: 10px 20px;border: none;border-radius: 5px;cursor: pointer;}input[type='submit']:hover {background-color: #45a049;}</style><script>function getCurrentURL() {return window.location.href + 'update';}function displayUpdateLink() {var updateLink = document.getElementById('updateLink');updateLink.setAttribute('href', getCurrentURL());updateLink.innerHTML = 'Over-The-Air Update';}window.onload = displayUpdateLink;</script></head><body><div class='container'><h1 class='heading'>Welcome to Weather Station</h1><p class='subheading'>This page is use to adjust Hijri date diffrence</p><div class='card'><form action='/update' method='post'><label for='value'>Difference (in days):</label><input type='number' name='eepromValue' value='" + String(eepromValue) + "' min='-2' max='2'><input type='submit' value='Update'></form><a id='updateLink' href=''></a></div></div></body></html>");
}

void handleUpdate(AsyncWebServerRequest *request) {
  if (request->hasParam("eepromValue", true)) {
    int newEEPROMValue = request->getParam("eepromValue", true)->value().toInt();
    if (newEEPROMValue != eepromValue) {
      EEPROM.put(0, newEEPROMValue);
      EEPROM.commit();
      eepromValue = newEEPROMValue;
      Serial.println("EEPROM value updated: " + String(eepromValue));
      convertGregorianToHijri(globalDateVariable, eepromValue);
    }
  }
  request->redirect("/");
}

void updateDateTime() {
  updateNTP(timeClient);
  struct tm *localTimeInfo = getFormattedTime(timeClient);
  char formattedTime[30];

  strftime(formattedTime, 30, "%a %b %d, %Y %I:%M:%S %p", localTimeInfo);
  Serial.println(formattedTime);

  strftime(formattedTime, 30, "%d-%m-%Y", localTimeInfo);
  strcpy(globalDateVariable, formattedTime);

  Serial.print("The Date is  ");
  Serial.println(globalDateVariable);

  strftime(formattedTime, 30, "%a %d-%m-%y %I:%M%P", localTimeInfo);

  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 0);
  lcd.print(formattedTime);
}

void StartScreen() {
  lcd.setCursor(0, 0);
  lcd.print("HTTP server started");
  delay(1000);
  lcd.clear();
  lcd.print(WiFi.localIP());
  lcd.print("/update");
  delay(1500);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Welcome to");
  lcd.setCursor(0, 1);
  lcd.print("Weather Station");
}



// WeatherData getWeatherData() {
//   String url = "https://api.openweathermap.org/data/2.5/weather?q=Karachi&appid=YOUR_API_KEY&units=metric";
  
//   WiFiClient client;
//   HTTPClient http;
//   http.begin(client, url);

  

//   int httpCode = http.GET();
//   if (httpCode == HTTP_CODE_OK) {
//     DynamicJsonDocument doc(1024);
//     deserializeJson(doc, http.getString());
//     WeatherData data;
//     data.temp = doc["main"]["temp"];
//     data.feels_like = doc["main"]["feels_like"];
//     data.temp_min = doc["main"]["temp_min"];
//     data.temp_max = doc["main"]["temp_max"];
//     data.pressure = doc["main"]["pressure"];
//     data.humidity = doc["main"]["humidity"];
//     data.wind_speed = doc["wind"]["speed"];
//     data.wind_deg = doc["wind"]["deg"];
//     data.sunrise = doc["sys"]["sunrise"];
//     data.sunset = doc["sys"]["sunset"];
    

//   lcd.setCursor(0, 2);
//   lcd.print(data.temp);

//     return data;
//   } else {
//     Serial.println("Error getting weather data");
//     return {};
//   }
// }




HijriDate convertGregorianToHijri(char *date, int adjustment) {
  String url = "http://api.aladhan.com/v1/gToH/" + String(date) + "?adjustment=" + String(adjustment);

  WiFiClient client;
  HTTPClient http;
  http.begin(client, url);

  int httpCode = http.GET();
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, http.getString());

      int day = doc["data"]["hijri"]["day"];
      int month = doc["data"]["hijri"]["month"]["number"];
      int year = doc["data"]["hijri"]["year"];
      String dateStr = doc["data"]["hijri"]["date"];
      String weekdayEn = doc["data"]["hijri"]["weekday"]["en"];
      String weekdayAr = doc["data"]["hijri"]["weekday"]["ar"];
      String monthEn = doc["data"]["hijri"]["month"]["en"];
      String monthAr = doc["data"]["hijri"]["month"]["ar"];
      String designationAbbreviated = doc["data"]["hijri"]["designation"]["abbreviated"];

      HijriDate hijriDate;
      hijriDate.day = day;
      hijriDate.month = month;
      hijriDate.year = year;
      strcpy(hijriDate.date, dateStr.c_str());
      strcpy(hijriDate.weekdayEn, weekdayEn.c_str());
      strcpy(hijriDate.weekdayAr, weekdayAr.c_str());
      strcpy(hijriDate.monthEn, monthEn.c_str());
      strcpy(hijriDate.monthAr, monthAr.c_str());
      strcpy(hijriDate.designationAbbreviated, designationAbbreviated.c_str());

      lcd.setCursor(0, 1);
      lcd.print("                    ");
      lcd.setCursor(0, 1);
       // Display the converted Hijri date

      Serial.print("Hijri Date: ");
      Serial.print(hijriDate.day);
      Serial.print("-");
      Serial.print(hijriDate.month);
      Serial.print("-");
      Serial.println(hijriDate.year);

      Serial.print("Date: ");
      Serial.println(hijriDate.date);

      Serial.print("Weekday (EN): ");
      Serial.println(hijriDate.weekdayEn);

      Serial.print("Weekday (AR): ");
      Serial.println(hijriDate.weekdayAr);

      Serial.print("Month (EN): ");
      Serial.println(hijriDate.monthEn);

      Serial.print("Month (AR): ");
      Serial.println(hijriDate.monthAr);

      Serial.print("Designation Abbreviated: ");
      Serial.println(hijriDate.designationAbbreviated);

      //lcd.print(dateStr + designationAbbreviated);

      lcd.print(formatWithLeadingZero(day) + " " + getIslamicMonthName(month) + " " + String(year) + " " + designationAbbreviated);
      return hijriDate;
    }
  }

  HijriDate emptyDate = {0, 0, 0, "", "", "", "", "", ""};
  return emptyDate;
}

String formatWithLeadingZero(int number) {
  if (number < 10) {
    return "0" + String(number);
  } else {
    return String(number);
  }
}

String getIslamicMonthName(int monthNumber) {
  String monthName;
  switch (monthNumber) {
    case 1:
      monthName = "Muharram";
      break;
    case 2:
      monthName = "Safar";
      break;
    case 3:
      monthName = "Rabi-I";
      break;
    case 4:
      monthName = "Rabi-II";
      break;
    case 5:
      monthName = "Jamadi-I";
      break;
    case 6:
      monthName = "Jamadi-II";
      break;
    case 7:
      monthName = "Rajab";
      break;
    case 8:
      monthName = "Shaban";
      break;
    case 9:
      monthName = "Ramadan";
      break;
    case 10:
      monthName = "Shawwal";
      break;
    case 11:
      monthName = "Zil-Qaad";
      break;
    case 12:
      monthName = "Zil-Hajj";
      break;
    default:
      monthName = "===";
      break;
  }
  return monthName;
}


