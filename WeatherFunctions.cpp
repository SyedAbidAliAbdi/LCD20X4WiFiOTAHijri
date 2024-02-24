
#include <ESP8266WiFi.h>
#include "WeatherFunctions.h"

String getWindDirection(float degrees);


// Function to get weather data from OpenWeatherMap API
WeatherData getWeatherData(const String& apiKey) {
  WeatherData data;

  // Check Wi-Fi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Not connected to Wi-Fi");
    return data;
  }

  // Make a request to OpenWeatherMap API
  WiFiClientSecure client;
  HTTPClient http;

  // Construct the URL with the API key and city (Karachi)
  String url = "https://api.openweathermap.org/data/2.5/weather?q=Karachi&appid=" + String(apiKey) + "&units=metric";

  Serial.print("Requesting URL: ");
  Serial.println(url);

  if (!client.connect("api.openweathermap.org", 80)) {
    Serial.println(client.connect("api.openweathermap.org", 80));
    Serial.println("Connection failed. Check DNS configuration or firewall settings");
    return data;
  }
  

  if (http.begin(client, url)) {
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();

      DynamicJsonDocument doc(2048);
      DeserializationError err = deserializeJson(doc, payload);

      if (err) {
        Serial.println("JSON parsing failed");
        return data;
      }

      // Extract weather data
      data.temp = doc["main"]["temp"];
      data.feels_like = doc["main"]["feels_like"];
      data.temp_min = doc["main"]["temp_min"];
      data.temp_max = doc["main"]["temp_max"];
      data.pressure = doc["main"]["pressure"];
      data.humidity = doc["main"]["humidity"];
      data.wind_speed = doc["wind"]["speed"];
      data.wind_deg = doc["wind"]["deg"];
      data.wind_direction = getWindDirection(doc["wind"]["deg"]);
      data.sunrise = doc["sys"]["sunrise"];
      data.sunset = doc["sys"]["sunset"];

      Serial.printf("Temperature = %.2f°C\r\n", data.temp);
      Serial.printf("Humidity = %d %%\r\n", data.humidity);
      Serial.printf("Pressure = %.3f bar\r\n", data.pressure);
      Serial.printf("Wind speed = %.1f m/s\r\n", data.wind_speed);
      Serial.printf("Wind degree = %d°\r\n\r\n", data.wind_deg);
    } else {
      Serial.printf("HTTP request failed with error code: %d\r\n", httpCode);
    }

    http.end();
  } else {
    Serial.println("Unable to begin HTTP request");
  }

  return data;
}


 

// Function to convert wind direction from degrees to NE format
String getWindDirection(float degrees) {
  if (degrees >= 348.75 || degrees < 11.25) {
    return "N";
  } else if (degrees >= 11.25 && degrees < 33.75) {
    return "NNE";
  } else if (degrees >= 33.75 && degrees < 56.25) {
    return "NE";
  } else if (degrees >= 56.25 && degrees < 78.75) {
    return "ENE";
  } else if (degrees >= 78.75 && degrees < 101.25) {
    return "E";
  } else if (degrees >= 101.25 && degrees < 123.75) {
    return "ESE";
  } else if (degrees >= 123.75 && degrees < 146.25) {
    return "SE";
  } else if (degrees >= 146.25 && degrees < 168.75) {
    return "SSE";
  } else if (degrees >= 168.75 && degrees < 191.25) {
    return "S";
  } else if (degrees >= 191.25 && degrees < 213.75) {
    return "SSW";
  } else if (degrees >= 213.75 && degrees < 236.25) {
    return "SW";
  } else if (degrees >= 236.25 && degrees < 258.75) {
    return "WSW";
  } else if (degrees >= 258.75 && degrees < 281.25) {
    return "W";
  } else if (degrees >= 281.25 && degrees < 303.75) {
    return "WNW";
  } else if (degrees >= 303.75 && degrees < 326.25) {
    return "NW";
  } else if (degrees >= 326.25 && degrees < 348.75) {
    return "NNW";
  } else {
    return "---";
  }
}
