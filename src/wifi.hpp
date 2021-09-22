#include <WiFi.h>
#include <esp_wifi.h>

#define WIFI_RETRY_CONNECTION 30  // 30 seconds wait for wifi connection

bool wifiCheck();
bool wifiInit();
void wifiStop();
void wifiRestart();
int  getWifiRSSI();

