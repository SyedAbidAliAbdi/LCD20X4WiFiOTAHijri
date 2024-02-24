#ifndef NTP_FUNCTIONS_H
#define NTP_FUNCTIONS_H

#include <NTPClient.h>
#include <WiFiUdp.h>

void setupNTP(WiFiUDP &ntpUDP, NTPClient &timeClient) {
  timeClient.begin();
}

void updateNTP(NTPClient &timeClient) {
  timeClient.update();
}

struct tm *getFormattedTime(NTPClient &timeClient) {
  unsigned long epochTime = timeClient.getEpochTime();
  time_t timeInfo = epochTime + 3600; // Add 5 hours for UTC+5
  struct tm *localTimeInfo = localtime(&timeInfo);
  return localTimeInfo;
}

#endif
