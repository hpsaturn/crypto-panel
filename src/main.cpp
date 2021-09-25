/*
 * Crypto Currency Monitor using ESP32 & E-Paper Display
 * =====================================================
 * 
 * Original project for Arduino IDE: 
 * https://www.youtube.com/techiesms
 * Author: techiesms
 * 
 * PlatformIO improvement
 * Author: @hpsaturn
 * 
 * Revision
 * --------------------------------------------------------
 * 0000 Exported credentials to globar vars on Pio project
 * 0001 Added status (battery) and some minors
 * 0002 Added FreeRTOS task for static GUI and complete refactor
 */

#ifndef BOARD_HAS_PSRAM
#error "Please enable PSRAM !!!"
#endif

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SD.h>
#include <SPI.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <wifi.hpp>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include "cryptos.h"
#include "coingecko-api.h"
#include "hal.h"
#include "settings.h"
// epd
#include "epd_driver.h"
#include "epd_highlevel.h"
#include "rom/rtc.h"
#include "eInkHandler.h"
#include "powertools.h"

// ----------------------------
// Configurations 
// ----------------------------

// default currency
const char *currency_base = "eur";

// ----------------------------
// End of area you need to change
// ----------------------------

// extra debug msgs
bool devmod = (bool)CORE_DEBUG_LEVEL;

#define MAX_RETRY 2     // max retry download
int retry;              // retry count

uint8_t *framebuffer;

// NTP time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
String formattedDate;
String dayStamp;
String timeStamp;
// GMT Offset in seconds. UK normal time is GMT, so GMT Offset is 0, for US (-5Hrs) is typically -18000.
int   gmtOffset_sec = 7200;

void title() {
    setFont(OpenSans24B);
    cursor_x = 20;
    cursor_y = TITLEY;
    String sym = "Coin";
    drawString(cursor_x, cursor_y, sym, LEFT);

    cursor_x = 220;
    cursor_y = TITLEY;
    String prc = "Price";
    drawString(cursor_x, cursor_y, prc, LEFT);

    cursor_x = 460;
    cursor_y = TITLEY;
    String da = "Day(%)";
    drawString(cursor_x, cursor_y, da, LEFT);

    cursor_x = 700;
    cursor_y = TITLEY;
    String we = "Week(%)";
    drawString(cursor_x, cursor_y, we, LEFT);

    fillRect(1, cursor_y+10, EPD_WIDTH, 5, Black);
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

String getFormatCurrencyValue(double value){
    char output[50];
    sprintf(output, "%05.2f",value);
    return String(output);
}

void renderCryptoCard(Crypto crypto) {
    if(devmod) Serial.printf("-->[eINK] Crypto Name  - %s\n",crypto.symbol.c_str());

    setFont(OpenSans14B);

    cursor_x = 50;
    char *string1 = &crypto.symbol[0];
    drawString(cursor_x,cursor_y,String(string1),LEFT);

    cursor_x = 220;
    String Str = getFormatCurrencyValue(crypto.price.inr);
    char *string2 = &Str[0];
    if(devmod) Serial.printf("-->[eINK] Price USD - %s\n",Str.c_str());
    drawString(cursor_x,cursor_y,String(string2),LEFT);

    if(devmod) Serial.printf("-->[eINK] Day change - %s\n",formatPercentageChange(crypto.dayChange).c_str());
    cursor_x = 480;
    Str = getFormatCurrencyValue(crypto.dayChange);
    char *string3 = &Str[0];
    drawString(cursor_x,cursor_y,String(string3),LEFT);

    if(devmod) Serial.printf("-->[eINK] Week change - %s\n",formatPercentageChange(crypto.weekChange).c_str());
    cursor_x = 700;
    Str = getFormatCurrencyValue(crypto.weekChange);
    char *string4 = &Str[0];
    drawString(cursor_x,cursor_y,String(string4),LEFT);

    drawFastHLine(1, cursor_y+10, EPD_WIDTH, Black);

}

void updateData() {
    if(devmod) Serial.println("-->[eINK] Rendering partial GUI..");
    for (int i = 0; i < cryptosCount; i++) {
        cursor_y = (45 * (i + 3));
        renderCryptoCard(cryptos[i]);
    }
    // clearStatusMsg();
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
    //fillRect(x, y - 1, 5, 1, GxEPD_BLACK);
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
    //drawString(x + 13, y + 5,  String(voltage, 2) + "v", CENTER);
}

void getNTPDateTime() {
    while (!timeClient.update()) {
        timeClient.forceUpdate();
    }
    formattedDate = timeClient.getFormattedDate();
    int splitT = formattedDate.indexOf("T");
    dayStamp = formattedDate.substring(0, splitT);
    timeStamp = formattedDate.substring(splitT + 1, formattedDate.length() - 1);
}

void displayGeneralInfoSection() {
    // Uncomment the next line if the display of IP- and MAC-Adddress is wanted
    //drawString(SCREEN_WIDTH - 150, 20, "IP=" + LocalIP + ",  MAC=" + WiFi.macAddress() ,RIGHT);
    // drawFastHLine(5, 30, SCREEN_WIDTH - 8, Black);
    String rev = " rev0" + String(REVISION);
    drawString(EPD_WIDTH-10, STATUSY, rev, RIGHT);
    getNTPDateTime();
    drawString(EPD_WIDTH / 2, 14, "Refreshed: " + dayStamp + " at " + timeStamp, CENTER);
}

void displayStatusSection() {
    drawBattery(5, 14);
    displayGeneralInfoSection();
    drawRSSI(850, 14, getWifiRSSI());
    fillRect(1, 16, EPD_WIDTH, 2, Black);
    fillRect(1, EPD_HEIGHT-39, EPD_WIDTH, 3, Black); 
    renderStatusMsg("Downloading Crypto data..");
}

void onUpdateMessage(const char *msg){
    renderStatusMsg("Updating firmware to rev 0"+String(msg));
    delay(100);
    setInt(key_boot_count, 0);
}

void renderVersion() {
    String status = ""+String(FLAVOR)+" v"+VERSION+" "+TARGET;
    renderStatusMsg(status);
}

void eInkTask(void* pvParameters) {

    eInkInit();
    
    int boot_count = getInt(key_boot_count, 0);
    Serial.printf("-->[eINK] boot_count: %i\n",boot_count);

    int reset_reason = rtc_get_reset_reason(0);
    if(devmod) Serial.printf("-->[eINK] reset_reason: %i\n",reset_reason);

    if(devmod) Serial.println("-->[eINK] Drawing static GUI..");
    if (boot_count == 0 || reset_reason == 1) {
        eInkClear();
        renderStatusMsg("LOADING...");
        title();
    }
    else renderStatusMsg("====== Connecting ======");
    
    displayStatusSection();

    if (boot_count++ > atoi(EDP_REFRESH_COUNT)) setInt(key_boot_count, 0);
    else setInt(key_boot_count, boot_count++);

    vTaskDelete(NULL);
}

void setupGUITask() {
    Serial.println("-->[eINK] Starting eINK Task..");
    xTaskCreatePinnedToCore(
        eInkTask,    /* Function to implement the task */
        "eInkTask ", /* Name of the task */
        10000,       /* Stack size in words */
        NULL,        /* Task input parameter */
        1,           /* Priority of the task */
        NULL,        /* Task handle. */
        1);          /* Core where the task should run */
}

bool downloadData() {    
    bool baseDataReady = downloadBaseData(currency_base);
    if(!baseDataReady){
        renderStatusMsg("== Bad response on BASE currency API ==");
        return false;
    }
    delay(100);
    bool cryptoDataReady = downloadBtcAndEthPrice();
    if(!baseDataReady){
        renderStatusMsg("== Bad response on Crypto currency API ==");
        return false;
    }
    
    if(baseDataReady && cryptoDataReady) renderVersion();
    return baseDataReady && cryptoDataReady;
}

void setup() {
    Serial.begin(115200);
    correct_adc_reference();
    setupGUITask();
    int boot_count = getInt(key_boot_count, 0);
    bool data_ready = false;

    if (wifiInit()) {
        otaMessageCb(&onUpdateMessage);
        timeClient.begin();
        timeClient.setTimeOffset(gmtOffset_sec);
        if (boot_count == 0) {  // only in the full refresh it have download retry
            while (!data_ready && retry++ < MAX_RETRY) data_ready = downloadData();
            if(data_ready) updateData();
        }
        else if (downloadData()) updateData();
        else epd_update();
    }
    else {
        renderStatusMsg("WiFi connection lost..");
    }
    epd_update();
    otaLoop();
    delay(200);
    suspendDevice();
}

void loop() {}

