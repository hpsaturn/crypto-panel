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
#include <HTTPClient.h>
#include <SD.h>
#include <SPI.h>
#include <wifi.hpp>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include "cryptos.h"
#include "coingecko-api.h"
#include "hal.h"
#include "powertools.h"
#include "settings.h"
// epd
#include "epd_driver.h"
#include "epd_highlevel.h"
#include "rom/rtc.h"
#include "eInkHandler.h"

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

void title() {
    cursor_x = 20;
    cursor_y = 50;
    String sym = "Symbol";
    drawString(cursor_x, cursor_y, sym, LEFT);

    cursor_x = 290;
    cursor_y = 50;
    String prc = "Price";
    drawString(cursor_x, cursor_y, prc, LEFT);

    cursor_x = 520;
    cursor_y = 50;
    String da = "Day(%)";
    drawString(cursor_x, cursor_y, da, LEFT);

    cursor_x = 790;
    cursor_y = 50;
    String we = "Week(%)";
    drawString(cursor_x, cursor_y, we, LEFT);
}

void status() {
    cursor_x = 20;
    String bat = "BAT: ";
    drawString(cursor_x, STATUSY, bat, LEFT);

    cursor_x = 920;
    String rev = "r0" + String(REVISION);
    drawString(cursor_x, STATUSY, rev, RIGHT);
}

void renderBatteryStatus() {
    cursor_x = 90;
    drawString(cursor_x,STATUSY,calcBatteryLevel(),LEFT);
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

    cursor_x = 50;
    char *string1 = &crypto.symbol[0];
    drawString(cursor_x,cursor_y,String(string1),LEFT);
    // writeln((GFXfont *)&FiraSans, string1, &cursor_x, &cursor_y, NULL);

    cursor_x = 220;
    String Str = getFormatCurrencyValue(crypto.price.inr);
    char *string2 = &Str[0];

    if(devmod) Serial.printf("-->[eINK] Price USD - %s\n",Str.c_str());

    drawString(cursor_x,cursor_y,String(string2),LEFT);
    // writeln((GFXfont *)&FiraSans, string2, &cursor_x, &cursor_y, NULL);

    if(devmod) Serial.printf("-->[eINK] Day change - %s\n",formatPercentageChange(crypto.dayChange).c_str());

    cursor_x = 530;
    Str = getFormatCurrencyValue(crypto.dayChange);
    char *string3 = &Str[0];
    drawString(cursor_x,cursor_y,String(string3),LEFT);
    // writeln((GFXfont *)&FiraSans, string3, &cursor_x, &cursor_y, NULL);

    if(devmod) Serial.printf("-->[eINK] Week change - %s\n",formatPercentageChange(crypto.weekChange).c_str());

    cursor_x = 800;
    Str = getFormatCurrencyValue(crypto.weekChange);
    char *string4 = &Str[0];
    drawString(cursor_x,cursor_y,String(string4),LEFT);
    // writeln((GFXfont *)&FiraSans, string4, &cursor_x, &cursor_y, NULL);
    epd_update();
}

void updateData() {
    if(devmod) Serial.println("-->[eINK] Rendering partial GUI..");
    for (int i = 0; i < cryptosCount; i++) {
        cursor_y = (50 * (i + 3));
        renderCryptoCard(cryptos[i]);
    }
    clearStatusMsg();
}

void eInkTask(void* pvParameters) {

    eInkInit();
    
    int boot_count = getInt(key_boot_count, 0);
    Serial.printf("-->[eINK] boot_count: %i\n",boot_count);

    int reset_reason = rtc_get_reset_reason(0);
    if(devmod) Serial.printf("-->[eINK] reset_reason: %i\n",reset_reason);

    if (boot_count == 0 || reset_reason == 1) eInkClear();
    if (boot_count++ > atoi(EDP_REFRESH_COUNT)) setInt(key_boot_count, 0);
    else setInt(key_boot_count, boot_count++);

    if(devmod) Serial.println("-->[eINK] Drawing static GUI..");

    title();
    setupBattery(); 
    status();
    renderBatteryStatus();
    renderStatusMsg("Downloading Crypto data..");
    epd_update();
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
    if(baseDataReady) renderStatusMsg("Base data downloaded");
    delay(100);
    bool cryptoDataReady = downloadBtcAndEthPrice();
    if(baseDataReady && cryptoDataReady) renderStatusMsg("Crypto data ready :D");
    return baseDataReady && cryptoDataReady;
}

void setup() {
    Serial.begin(115200);

    setupGUITask();

    int boot_count = getInt(key_boot_count, 0);
    bool data_ready = false;

    if (wifiInit()) {
        if (boot_count == 0) {  // only in the full refresh it have download retry
            while (!data_ready && retry++ < MAX_RETRY) data_ready = downloadData();
            if(data_ready) updateData();
        }
        else if (downloadData()) updateData();
    }

    epd_poweroff();
    suspendDevice();
}

void loop() {}

