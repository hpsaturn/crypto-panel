#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFiClientSecure.h>
#include <esp_wifi.h>
#include <OTAHandler.h>

#define WIFI_RETRY_CONNECTION 30  // 30 seconds wait for wifi connection

void otaLoop();
bool wifiCheck();
bool wifiInit();
void wifiStop();
void wifiRestart();
int  getWifiRSSI();

void otaMessageCb(voidMessageCbFn cb);