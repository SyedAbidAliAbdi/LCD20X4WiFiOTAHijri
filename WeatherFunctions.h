#ifndef WEATHER_FUNCTIONS_H
#define WEATHER_FUNCTIONS_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

String getWindDirection(float degrees); // Function declaration
// Define a structure to hold weather data
struct WeatherData {
  float temp;
  float feels_like;
  float temp_min;
  float temp_max;
  float pressure;
  float humidity;
  float wind_speed;
  float wind_deg;
  String wind_direction;
  unsigned long sunrise;
  unsigned long sunset;
};

// Function to get weather data from OpenWeatherMap API
WeatherData getWeatherData(const String& apiKey);

#endif
