#include "esp_adc_cal.h"

// power consumption settings
#define DEEP_SLEEP_DURATION 60  // sleep x seconds and then wake up
#define MAX_REFRESH_COUNT 30     // boot counts to complete clean screen

int vref = 1100;

void suspendDevice() {
    Serial.println("-->[eINK] shutdown..");
    esp_sleep_enable_timer_wakeup(1000000LL * DEEP_SLEEP_DURATION);
    esp_deep_sleep_start();
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
        log_i("eFuse Vref:%u mV\n", adc_chars.vref);
        vref = adc_chars.vref;
    }
}

String calcBatteryLevel() {
    uint16_t v = analogRead(BATT_PIN);
    float battery_voltage = ((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
    String voltage = String(battery_voltage) + "v";
    Serial.printf("-->[BATT] %s\n",voltage.c_str());
    return voltage;
}