#include <WiFi.h>
#include <OTAHandler.h>
#include <esp_wifi.h>

#define WIFI_RETRY_CONNECTION 30  // 30 seconds wait for wifi connection

void otaLoop();
bool wifiCheck();
bool wifiInit();
void wifiStop();
void wifiRestart();
int  getWifiRSSI();

void otaMessageCb(voidMessageCbFn cb);