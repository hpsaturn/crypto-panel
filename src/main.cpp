/*
 * Crypto Currency Monitor using ESP32 & E-Paper Display
 * =====================================================
 *
 * Original project for Arduino IDE:
 * https://www.youtube.com/techiesms
 * Author: techiesms
 *
 * PlatformIO improvement and Crypto News API integration:
 * https://github.com/hpsaturn/crypto-currency
 * Author: @hpsaturn
 * 2021
 */

#ifndef BOARD_HAS_PSRAM
#error "Please enable PSRAM !!!"
#endif

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <wifi.hpp>
#include "epd_driver.h"
#include "epd_highlevel.h"
#include "rom/rtc.h"
#include "hal.h"
#include "models.h"
#include "apis.h"
#include "settings.h"
#include "powertools.h"
#include "guitools.h"

#define MAX_RETRY 2  // max retry download

String currency_base = "eur"; // default currency. Please change this value via CLI.
bool devmod = (bool)CORE_DEBUG_LEVEL;  // extra debug msgs
bool BtnConfigPressed;
bool inSetup;

// NTP time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
String formattedDate;
String dayStamp;
String timeStamp;
int gmtOffset_sec = 7200;  // GMT Offset in seconds. GMT Offset is 0, US (-5Hrs) is typically -18000.

void logMemory() {
  if (devmod) Serial.printf("-->[IHAL] Used PSRAM: %d\r\n", ESP.getPsramSize() - ESP.getFreePsram());
}

bool isConfigured() {
    return wcli.isConfigured() && cryptosCount == 3;
}

void title() {
  setFont(OpenSans24B);
  String sym = "Coin";
  drawString(20, TITLEY, sym, LEFT);

  String prc = "Price";
  drawString(220, TITLEY, prc, LEFT);

  String da = "Day(%)";
  drawString(460, TITLEY, da, LEFT);

  String we = "Week(%)";
  drawString(700, TITLEY, we, LEFT);

  fillRect(1, TITLEY + 10, EPD_WIDTH, 5, Black);
}

String formatPercentageChange(double change) {
  double absChange = change;
  if (change < 0) absChange = -change;

  if (absChange > 100) {
    return String(absChange, 0) + "%";
  } else if (absChange >= 10) {
    return String(absChange, 1) + "%";
  } else {
    return String(absChange) + "%";
  }
}

String getFormatCurrencyValue(double value) {
  char output[50];
  sprintf(output, "%04.2f", value);
  return String(output);
}

void renderCryptoCard(Crypto crypto) {
  if (devmod) Serial.printf("-->[eINK] Crypto Name - %s\r\n", crypto.symbol.c_str());

  setFont(OpenSans14B);

  char *string1 = &crypto.symbol[0];
  drawString(50, cursor_y, String(string1), LEFT);

  String Str = getFormatCurrencyValue(crypto.price.inr);
  char *string2 = &Str[0];
  if (devmod) Serial.printf("-->[eINK] Price USD - %s\r\n", Str.c_str());
  drawString(345, cursor_y, String(string2), RIGHT);

  if (devmod) Serial.printf("-->[eINK] Day change - %s\r\n", formatPercentageChange(crypto.dayChange).c_str());
  Str = getFormatCurrencyValue(crypto.dayChange);
  char *string3 = &Str[0];
  drawString(560, cursor_y, String(string3), RIGHT);

  if (devmod) Serial.printf("-->[eINK] Week change - %s\r\n", formatPercentageChange(crypto.weekChange).c_str());
  Str = getFormatCurrencyValue(crypto.weekChange);
  char *string4 = &Str[0];
  drawString(EPD_WIDTH - 130, cursor_y, String(string4), RIGHT);

  drawFastHLine(1, cursor_y + 10, EPD_WIDTH, Black);
}

void updateData() {
  if (devmod) Serial.println("-->[eINK] Rendering partial GUI..");
  for (int i = 0; i < cryptosCount; i++) {
    cursor_y = (45 * (i + 3));
    renderCryptoCard(cryptos[i]);
  }
}

void displayDebugInfo() {
  int reset_count = getInt(key_boot_count, 0);
  String netip = "IP: " + WiFi.localIP().toString();
  String netmc = "MAC:" + WiFi.macAddress();
  String boots = "Weakup count: " + String(reset_count) + "/" + String(EPD_REFRESH_COUNT);
  String confg = "Deep sleep duration: " + String(deep_sleep_time) + " seconds";
  String wakup = get_wakeup_reason();
  String reset = get_reset_reason(0);

  drawString(DEBUGIX, DEBUGIY - DEBUGIH * 6, netip, RIGHT);
  drawString(DEBUGIX, DEBUGIY - DEBUGIH * 5, netmc, RIGHT);
  drawString(DEBUGIX, DEBUGIY - DEBUGIH * 4, boots, RIGHT);
  drawString(DEBUGIX, DEBUGIY - DEBUGIH * 3, confg, RIGHT);
  drawString(DEBUGIX, DEBUGIY - DEBUGIH * 2, wakup, RIGHT);
  drawString(DEBUGIX, DEBUGIY - DEBUGIH * 1, reset, RIGHT);
}

void drawRSSI(int x, int y, int rssi) {
  int WIFIsignal = 0;
  int xpos = 1;
  for (int _rssi = -100; _rssi <= rssi; _rssi = _rssi + 20) {
    if (_rssi <= -20) WIFIsignal = 30;  //            <-20dbm displays 5-bars
    if (_rssi <= -40) WIFIsignal = 24;  //  -40dbm to  -21dbm displays 4-bars
    if (_rssi <= -60) WIFIsignal = 18;  //  -60dbm to  -41dbm displays 3-bars
    if (_rssi <= -80) WIFIsignal = 12;  //  -80dbm to  -61dbm displays 2-bars
    if (_rssi <= -100) WIFIsignal = 6;  // -100dbm to  -81dbm displays 1-bar
    fillRect(x + xpos * 8, y - WIFIsignal, 7, WIFIsignal, Black);
    xpos++;
  }
  // fillRect(x, y - 1, 5, 1, GxEPD_BLACK);
  drawString(x + 40, y, String(rssi) + "dBm", LEFT);
}

void drawBattery(int x, int y) {
  setFont(OpenSans8B);
  double_t percentage = get_battery_percentage();
  drawRect(x + 55, y - 15, 40, 15, Black);
  fillRect(x + 95, y - 9, 4, 6, Black);
  fillRect(x + 57, y - 13, 36 * percentage / 100.0, 11, Black);
  drawString(x, y, String((int)percentage) + "%", LEFT);
  drawString(10, STATUSY, "Batt:" + String(battery_voltage) + "v", LEFT);
}

void getNTPDateTime() {
  int retry = 0;
  while (!timeClient.update() && retry++ < MAX_RETRY * 3) timeClient.forceUpdate();
  formattedDate = timeClient.getFormattedDate();
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  timeStamp = formattedDate.substring(splitT + 1, formattedDate.length() - 1);
}

void displayGeneralInfoSection() {
  // Uncomment the next line if the display of IP- and MAC-Adddress is wanted
  // drawFastHLine(5, 30, SCREEN_WIDTH - 8, Black);
  String rev = "rev" + String(REVISION);
  drawString(EPD_WIDTH - 10, STATUSY, rev, RIGHT);
  getNTPDateTime();
  drawString(EPD_WIDTH / 2, 14, "Refreshed: " + dayStamp + " at " + timeStamp, CENTER);
}

void displayStatusSection() {
  drawBattery(5, 14);
  displayGeneralInfoSection();
  drawRSSI(850, 14, getWifiRSSI());
  if (devmod) displayDebugInfo();
  fillRect(1, 16, EPD_WIDTH, 2, Black);
  fillRect(1, EPD_HEIGHT - 39, EPD_WIDTH, 3, Black);
  if(inSetup) renderStatusMsg("waiting for configuration..");
  else renderStatusMsg("Downloading Crypto data.."); 
}

void onUpdateMessage(const char *msg) {
  renderStatusMsg("Updating firmware to rev 0" + String(msg) + "..");
  delay(200);
  setInt(key_boot_count, 0);
}

void renderStaticContent(bool inSetup){
    eInkClear();
    renderStatusMsg("LOADING...");
    if(inSetup) renderPost("Welcome to Cryptocurrency Panel!", "Please enter to the serial console to setup");
    else title();
}

void eInkTask(void *pvParameters) {
  eInkInit();
  logMemory();

  int boot_count = getInt(key_boot_count, 0);
  if(devmod) Serial.printf("-->[eINK] boot_count: %i\r\n", boot_count);

  int reset_reason = rtc_get_reset_reason(0);
  if (devmod) Serial.printf("-->[eINK] reset_reason: %i\r\n", reset_reason);
  
  if (devmod) Serial.println("-->[eINK] Drawing static GUI..");
  if (boot_count == 0 || reset_reason == 1 || inSetup)
    renderStaticContent(inSetup);
  else
    renderStatusMsg("========= Connecting =========");

  if (boot_count++ > EPD_REFRESH_COUNT || inSetup)
    setInt(key_boot_count, 0);
  else
    setInt(key_boot_count, boot_count);

  displayStatusSection();

  vTaskDelete(NULL);
}

void setupGUITask() {
  if (devmod) Serial.println("\r\n-->[eINK] Starting eINK Task..");
  xTaskCreatePinnedToCore(
      eInkTask,    /* Function to implement the task */
      "eInkTask ", /* Name of the task */
      10000,       /* Stack size in words */
      NULL,        /* Task input parameter */
      1,           /* Priority of the task */
      NULL,        /* Task handle. */
      1);          /* Core where the task should run */
}

void renderVersion() {
  String status = "" + String(FLAVOR) + " v" + VERSION + " " + TARGET;
  renderStatusQueue(status);
}

void renderNetworkError() {
  String status = "Last message: Bad response from Crypto API";
  renderStatusQueue(status);
}

void renderNews() {
  if (devmod) Serial.println("-->[nAPI] News Author: " + news.author);

  uint32_t qrlenght = news.qrsize * news.qrsize;
  uint8_t *buffer = (uint8_t *)ps_malloc(qrlenght / 2);
  for (unsigned i = 0, uchr; i < qrlenght; i += 2) {
    sscanf(news.qr + i, "%2x", &uchr);  // conversion
    buffer[i / 2] = uchr;               // save as char
  }
  renderPost(news.title, news.summary, news.published, news.author, buffer, news.qrsize);
  free(buffer);
}

bool downloadData() {
  bool baseDataReady = downloadBaseData(currency_base);
  delay(100);
  bool cryptoDataReady = downloadBtcAndEthPrice();
  delay(100);
  if (downloadNewsData()) renderNews();

  bool success = baseDataReady && cryptoDataReady;

  if (success) renderVersion();
  return success;
}

void printRequirements() {
  if (cryptosCount < 3) {
    Serial.println("\r\nPlease enter at least 3 currencies to finish setup");
  }
  if (!wcli.isConfigured()) {
    Serial.println("\r\nPlease configure at least one WiFi to finish setup");
  }
}

class mESP32WifiCLICallbacks : public ESP32WifiCLICallbacks {
  void onWifiStatus(bool isConnected) {
  }

  // Callback for extend the help menu.
  void onHelpShow() {
    Serial.println("\r\nCrypto Panel Commands:\r\n");
    Serial.println("curAdd <crypto>   \tadd one cryptocurrency");
    Serial.println("curList\t\t\tlist saved cryptocurrencies");
    Serial.println("curDrop <crypto> \tdelete one cryptocurrency");
    Serial.println("setBase <base>\t\tset base currency (USD/EUR)");
    Serial.println("setSleep <time>   \tconfig deep sleep time in minutes");
    Serial.println("setTemp <temperature>\tconfig the panel ambient temperature");
    Serial.println("reboot\t\t\tperform a soft ESP32 reboot");
    Serial.println("help\t\t\tdisplay this help menu\r\n");
    printRequirements();
  }
};

void _setBase (String opts) {
  maschinendeck::Pair<String, String> operands = maschinendeck::SerialTerminal::ParseCommand(opts);
  String base = operands.first();
  base.toLowerCase();
  if (base.equals("usd") || base.equals("eur")) {
    currency_base = base;
    setString(key_cur_base, base);
  } else {
    Serial.println("\r\nInvalid base currency. Please enter USD or EUR");
    Serial.printf("Current base currency: %s\r\n", currency_base.c_str());
  }
}

void _setSleepTime(String opts) {
  maschinendeck::Pair<String, String> operands = maschinendeck::SerialTerminal::ParseCommand(opts);
  int sleepTime = operands.first().toInt();
  if (sleepTime >= 5) {
    deep_sleep_time = sleepTime * 60;
    setInt(key_sleep_time, sleepTime);
  } else {
    Serial.printf("\r\ninvalid sleep time\r\ncurrent sleep time is: %i\r\n",deep_sleep_time / 60);
    Serial.println("minimum sleep time is 5 minutes. Recommended is 60 minutes or more");
  }
}

void _setTemp(String opts) {
  maschinendeck::Pair<String, String> operands = maschinendeck::SerialTerminal::ParseCommand(opts);
  int temp = operands.first().toInt();
  if (temp < 10 || temp > 50) {
    Serial.println("\r\nplease enter a temperature value between 10 and 50");
    Serial.printf("current temperature is: %d\r\n",ambient_temp);
    return;
  }
  ambient_temp = temp;
  setInt(key_panel_temp, temp);
}

void _cryptoList(String opts){
  listCryptos();
}

void _cryptoDelete(String opts){
  maschinendeck::Pair<String, String> operands = maschinendeck::SerialTerminal::ParseCommand(opts);
  String crypto = operands.first();
  deleteCrypto(crypto);
}

void _cryptoSave(String opts) {
  maschinendeck::Pair<String, String> operands = maschinendeck::SerialTerminal::ParseCommand(opts);
  String crypto = operands.first();
  saveCrypto(crypto);
}

void reboot(String opts){
  ESP.restart();
}

void setupWiFiCLI(){
  if (inSetup) {
    Serial.println("\r\n== Setup Mode ==\r\n");
    printRequirements();
    Serial.flush();
  }
  wcli.setCallback(new mESP32WifiCLICallbacks());
  wcli.setSilentMode(true);
  wcli.begin();
  wcli.term->add("curAdd", &_cryptoSave, "\tadd one cryptocurrency. Max 3");
  wcli.term->add("curList", &_cryptoList, "\tlist saved cryptocurrencies");
  wcli.term->add("curDrop", &_cryptoDelete, "\tdelete one cryptocurrency");
  wcli.term->add("setBase", &_setBase, "\tset base currency (USD/EUR)");
  wcli.term->add("setSleep", &_setSleepTime, "config deep sleep time");
  wcli.term->add("setTemp", &_setTemp, "\tconfig the panel ambient temperature");
  wcli.term->add("reboot", &reboot, "\tperform panel reboot\r\n");

  while (!isConfigured() || BtnConfigPressed) {  // force to configure the panel.
    wcli.loop();
  }
  if(inSetup) renderStaticContent(false);  // restore normal static content
}

void setupFlags(){
  listCryptos(true);  // load configured crypto currencies from flash
  BtnConfigPressed = (digitalRead(SETUP_BTN_PIN) == LOW || wakeup_by_setup_button());
  inSetup = !isConfigured() || BtnConfigPressed;
  currency_base = getString(key_cur_base, "eur");
  ambient_temp = getInt(key_panel_temp, 22);
  deep_sleep_time = getInt(key_sleep_time, 10) * 60;
}

void setupWatchdog(){
  esp_task_wdt_init(40, true);
  esp_task_wdt_add(NULL);
}

void setup() {
  correct_adc_reference();
  pinMode(SETUP_BTN_PIN, INPUT_PULLUP);
  Serial.begin(115200);

  setupFlags();
  setupGUITask();
  setupWiFiCLI();
  setupWatchdog();
  
  Serial.println("\r\n== Setup ready ==\r\n");

  int boot_count = getInt(key_boot_count, 0);
  bool data_ready = false;

  if (wifiInit()) {
    otaMessageCb(&onUpdateMessage);
    timeClient.begin();
    timeClient.setTimeOffset(gmtOffset_sec);

    int retry = 0;
    if (boot_count == 0) {  // Only in the full refresh it does retry download
      while (!data_ready && retry++ < MAX_RETRY) data_ready = downloadData();
      if (data_ready)
        updateData();
      else
        renderNetworkError();
    } else if (downloadData())
      updateData();
    else
      renderNetworkError();
  } else {
    delay(100);
    // last try after to full clear
    if (boot_count == 0 && wifiInit() && downloadData())
      updateData();
    else
      renderStatusMsg("Last message: WiFi connection lost");
  }
  epd_update();
  logMemory();
  otaLoop();
  suspendDevice();
}

void loop() {}
