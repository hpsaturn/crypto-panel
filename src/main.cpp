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
#include <Preferences.h>
#include <esp_task_wdt.h>

#include "cryptos.h"
#include "coingecko-api.h"
#include "epd_driver.h"
#include "esp_adc_cal.h"
#include "firasans.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal.h"
#include "rom/rtc.h"

// ----------------------------
// Configurations 
// ----------------------------

// power consumption settings
#define DEEP_SLEEP_DURATION 30  // sleep x seconds and then wake up
#define MAX_REFRESH_COUNT 2     // boot counts to complete clean screen

// default currency
const char *currency_base = "eur";

// ----------------------------
// End of area you need to change
// ----------------------------

int cursor_x;
int cursor_y;

uint8_t *framebuffer;
int vref = 1100;
Preferences preferences;
const char* app_name = "crypto_currency";
const char* key_boot_count = "key_boot_count";

void title() {
    cursor_x = 20;
    cursor_y = 50;
    const char *sym = "Symbol";
    writeln((GFXfont *)&FiraSans, sym, &cursor_x, &cursor_y, NULL);

    cursor_x = 290;
    cursor_y = 50;
    const char *prc = "Price";
    writeln((GFXfont *)&FiraSans, prc, &cursor_x, &cursor_y, NULL);

    cursor_x = 520;
    cursor_y = 50;
    const char *da = "Day(%)";
    writeln((GFXfont *)&FiraSans, da, &cursor_x, &cursor_y, NULL);

    cursor_x = 790;
    cursor_y = 50;
    const char *we = "Week(%)";
    writeln((GFXfont *)&FiraSans, we, &cursor_x, &cursor_y, NULL);
}

String calcBatteryLevel() {
    uint16_t v = analogRead(BATT_PIN);
    float battery_voltage = ((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
    String voltage = String(battery_voltage) + "v";
    Serial.printf("-->[BATT] voltage: %s\n",voltage.c_str());
    return voltage;
}

void status() {
    cursor_x = 20;
    cursor_y = 500;
    const char *bat = "BAT: ";
    writeln((GFXfont *)&FiraSans, bat, &cursor_x, &cursor_y, NULL);
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

void renderCryptoCard(Crypto crypto) {
    Serial.print("-->[eINK] Crypto Name  - ");
    Serial.println(crypto.symbol);

    cursor_x = 50;

    char *string1 = &crypto.symbol[0];

    writeln((GFXfont *)&FiraSans, string1, &cursor_x, &cursor_y, NULL);

    cursor_x = 220;

    String Str = (String)(crypto.price.inr);
    char *string2 = &Str[0];

    Serial.print("-->[eINK] Price USD - ");
    Serial.println(Str);

    Rect_t area = {
        .x = cursor_x,
        .y = cursor_y - 40,
        .width = 320,
        .height = 50,
    };

    epd_clear_area(area);

    writeln((GFXfont *)&FiraSans, string2, &cursor_x, &cursor_y, NULL);

    Serial.print("-->[eINK] Day change - ");
    Serial.println(formatPercentageChange(crypto.dayChange));

    cursor_x = 530;

    Rect_t area1 = {
        .x = cursor_x,
        .y = cursor_y - 40,
        .width = 150,
        .height = 50,
    };

    epd_clear_area(area1);
    Str = (String)(crypto.dayChange);
    char *string3 = &Str[0];

    writeln((GFXfont *)&FiraSans, string3, &cursor_x, &cursor_y, NULL);

    Serial.print("-->[eINK] Week change - ");
    Serial.println(formatPercentageChange(crypto.weekChange));

    cursor_x = 800;

    Rect_t area2 = {
        .x = cursor_x,
        .y = cursor_y - 40,
        .width = 150,
        .height = 50,
    };

    epd_clear_area(area2);

    Str = (String)(crypto.weekChange);
    char *string4 = &Str[0];

    writeln((GFXfont *)&FiraSans, string4, &cursor_x, &cursor_y, NULL);
}

void renderStatus() {
    cursor_x = 110;
    cursor_y = 500;

    Rect_t area = {
        .x = cursor_x,
        .y = cursor_y - 40,
        .width = 150,
        .height = 50,
    };

    epd_clear_area(area);
    writeln((GFXfont *)&FiraSans, calcBatteryLevel().c_str(), &cursor_x, &cursor_y, NULL);
}

void setInt(String key, int value){
    preferences.begin(app_name, false);
    preferences.putInt(key.c_str(), value);
    preferences.end();
}

int32_t getInt(String key, int defaultValue){
    preferences.begin(app_name, false);
    int32_t out = preferences.getInt(key.c_str(), defaultValue);
    preferences.end();
    return out;
}

void espShallowSleep(int ms) {
    // commented it for possible fix for issue: https://github.com/Xinyuan-LilyGO/TTGO-T-Display/issues/36
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
    esp_sleep_enable_timer_wakeup(ms * 1000);
    delay(200);
    esp_light_sleep_start();
}

void setupBattery() {
    // Correct the ADC reference voltage
    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        Serial.printf("-->[BATT] eFuse Vref:%u mV\n", adc_chars.vref);
        vref = adc_chars.vref;
    }
}

void eInkTask(void* pvParameters) {
    
    epd_init();

    framebuffer = (uint8_t *)ps_calloc(sizeof(uint8_t), EPD_WIDTH * EPD_HEIGHT / 2);
    if (!framebuffer) {
        Serial.println("-->[eINK] alloc memory failed !!!");
        while (1);
    }
    memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);

    epd_poweron();

    int reset_reason = rtc_get_reset_reason(0);
    Serial.printf("-->[eINK] reset_reason: %i\n",reset_reason);
    if(reset_reason == 1 ) epd_clear();

    int boot_count = getInt(key_boot_count, 0);
    Serial.printf("-->[eINK] boot_count: %i\n",boot_count);

    if (boot_count == 0) epd_clear();
    if (boot_count++ > MAX_REFRESH_COUNT) setInt(key_boot_count, 0);
    else setInt(key_boot_count, boot_count++);

    epd_poweroff();
    epd_poweron();   
    Serial.println("-->[eINK] Drawing static GUI..");
    title();
    status();
    vTaskDelete(NULL);
}

void setupGUITask() {
    Serial.println("-->[eINK] Starting eINK Task..");
    xTaskCreatePinnedToCore(
        eInkTask,    /* Function to implement the task */
        "eInkTask ", /* Name of the task */
        10000,        /* Stack size in words */
        NULL,        /* Task input parameter */
        1,           /* Priority of the task */
        NULL,    /* Task handle. */
        1);          /* Core where the task should run */
}

void setup() {
    Serial.begin(115200);
    setupGUITask();
    wifiInit();
    setupBattery(); 
}

void loop() {
    downloadBaseData(currency_base);
    delay(100);
    downloadBtcAndEthPrice();
    Serial.println("-->[eINK] Rendering partial GUI..");
    for (int i = 0; i < cryptosCount; i++) {
        cursor_y = (50 * (i + 3));
        renderCryptoCard(cryptos[i]);
    }
    renderStatus();
    Serial.print("-->[eINK] shutdown..");
    epd_poweroff_all();
    esp_sleep_enable_timer_wakeup(1000000LL * DEEP_SLEEP_DURATION);
    esp_deep_sleep_start();
    Serial.print("done");
}