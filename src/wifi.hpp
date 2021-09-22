#include <WiFi.h>
#include <esp_wifi.h>

#define PUBLISH_INTERVAL 30       // publish to cloud each 30 seconds
#define WIFI_RETRY_CONNECTION 30  // 30 seconds wait for wifi connection
#define IFX_RETRY_CONNECTION 5    // influxdb publish retry 

bool wifiCheck();
void wifiInit();
void wifiStop();
void wifiRestart();
int  getWifiRSSI();

