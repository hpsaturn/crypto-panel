#include <wifi.hpp>

int rssi = 0;

/******************************************************************************
 *   W I F I   M E T H O D S
 ******************************************************************************/

void otaMessageCb(voidMessageCbFn cb) {
  ota.setOnUpdateMessageCb(cb);
}

void otaLoop() {
  if (WiFi.isConnected()) {
    ota.loop();
    ota.checkRemoteOTA();
  }
}

void otaInit() {
  ota.setup("EPD47ESP32", "epd47esp32");
}

void ntpInit() {
  configTime(0, 0, NTP_SERVER);
  setenv("TZ", TZ_INFO, 1);
}

bool wifiInit() {
  bool wifi_connected = WiFi.status() == WL_CONNECTED;
  if (wifi_connected) {
    otaInit();
    ntpInit();
  }
  return wifi_connected;
}

int getWifiRSSI() {
  if (WiFi.isConnected())
    return WiFi.RSSI();
  else
    return 0;
}
