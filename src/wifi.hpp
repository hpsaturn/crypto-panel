#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <WiFiClientSecure.h>
#include <esp_wifi.h>
#include <OTAHandler.h>
#include <ESP32WifiCLI.hpp>

#define WIFI_RETRY_CONNECTION 30                                    // 30 seconds wait for wifi connection
#define TZ_INFO    "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00"   // enter your custom time zone via CLI
#define NTP_SERVER "ch.pool.ntp.org"

void otaLoop();
bool wifiCheck();
bool wifiInit();
void wifiStop();
int  getWifiRSSI();

void otaMessageCb(voidMessageCbFn cb);