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

bool wifiConnect(const char* ssid, const char* pass) {
    Serial.printf("-->[WIFI] connecting to %s\n",ssid);
    int wifi_retry = 0;
    WiFi.begin(ssid, pass);
    while (!WiFi.isConnected() && wifi_retry++ <= WIFI_RETRY_CONNECTION) {
        delay(250);  // increment this delay on possible reconnect issues
    }
    delay(100);
    if (WiFi.isConnected()) {
        Serial.print("-->[WIFI] IP: ");
        Serial.println(WiFi.localIP());
        otaInit();
        return true;
    } else {
        Serial.println("-->[WIFI] disconnected!");
        return false;
    }
}

class mESP32WifiCLICallbacks : public ESP32WifiCLICallbacks {
  void onWifiStatus(bool isConnected) {
    
  }

  // Callback for extend the help menu.
  void onHelpShow() {
    Serial.println("\r\nCustom commands:\r\n");
    Serial.println("blink <times> <millis>\tLED blink x times each x millis");
    Serial.println("echo \"message\"\t\tEcho the input message");
    Serial.println("reboot\t\t\tperform a soft ESP32 reboot");
  }
};

bool wifiInit() {
    wcli.setCallback(new mESP32WifiCLICallbacks());
    wcli.begin();
    return wcli.wifiValidation();
}

void wifiStop() {
    Serial.println("-->[WIFI] Disconnecting..");
    WiFi.disconnect(true);
    delay(100);
}

void wifiRestart() {
    wifiStop();
    wifiInit();
}

int getWifiRSSI() {
    if (WiFi.isConnected()) return WiFi.RSSI();
    else return 0;
}

