#include <driver/adc.h>
#include "esp_adc_cal.h"
#include "esp_sleep.h"

#define BATTERY_MIN_V 3.2
#define BATTERY_MAX_V 4.1
#define BATTCHARG_MIN_V 4.65
#define BATTCHARG_MAX_V 4.88

int vref = 1100;
float curv = 0;

void suspendDevice() {
    Serial.println("-->[eINK] shutdown..");
    esp_sleep_enable_timer_wakeup(1000000LL * atoi(DEEP_SLEEP_TIME));
    esp_deep_sleep_start();
}

void espShallowSleep(int ms) {
    // commented it for possible fix for issue: https://github.com/Xinyuan-LilyGO/TTGO-T-Display/issues/36
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
    esp_sleep_enable_timer_wakeup(ms * 1000);
    delay(100);
    esp_light_sleep_start();
}

void setupBattery() {
    // Correct the ADC reference voltage
    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        log_i("eFuse Vref:%u mV", adc_chars.vref);
        vref = adc_chars.vref;
    } 
}

float calcBatteryLevel() {
    uint16_t v = analogRead(BATT_PIN);
    float battery_voltage = ((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
    return battery_voltage;
}

uint8_t _calcPercentage(float volts, float max, float min) {
    float percentage = (volts - min) * 100 / (max - min);
    if (percentage > 100) {
        percentage = 100;
    }
    if (percentage < 0) {
        percentage = 0;
    }
    return (uint8_t)percentage;
}

bool battIsCharging() {
    return curv > BATTERY_MAX_V + (BATTCHARG_MIN_V - BATTERY_MAX_V ) / 2;
}

uint8_t battCalcPercentage(float volts) {
    if (battIsCharging()){
      return _calcPercentage(volts,BATTCHARG_MAX_V,BATTCHARG_MIN_V);
    } else {
      return _calcPercentage(volts,BATTERY_MAX_V,BATTERY_MIN_V);
    }
}
