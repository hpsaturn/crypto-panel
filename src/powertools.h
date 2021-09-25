#include <driver/adc.h>
#include "esp_adc_cal.h"
#include "esp_sleep.h"

#define BATTERY_MIN_V 3.2
#define BATTERY_MAX_V 4.1
#define BATTCHARG_MIN_V 4.65
#define BATTCHARG_MAX_V 4.88

int vref = 1100;
float curv = 0;
double_t battery_voltage = 0;

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

void correct_adc_reference() {
    // Correct the ADC reference voltage
    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        Serial.printf("eFuse Vref:%u mV", adc_chars.vref);
        vref = adc_chars.vref;
    }
}

double_t get_battery_percentage() {
    // When reading the battery voltage, POWER_EN must be turned on
    epd_poweron();
    delay(50);

    Serial.println(epd_ambient_temperature());

    uint16_t v = analogRead(BATT_PIN);
    battery_voltage = ((double_t)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
    Serial.println("-->[vADC] " + String(battery_voltage) + "v");

    // Better formula needed I suppose
    // experimental super simple percent estimate no lookup anything just divide by 100
    double_t percent_experiment = ((battery_voltage - 3.7) / 0.5) * 100;

    // cap out battery at 100%
    // on charging it spikes higher
    if (percent_experiment > 100) {
        percent_experiment = 100;
    }

    String voltage = "-->[vADC] V which is around " + String(percent_experiment) + "%";
    Serial.println(voltage);

    epd_poweroff();
    delay(50);

    return percent_experiment;
}

bool battIsCharging() {
    return curv > BATTERY_MAX_V + (BATTCHARG_MIN_V - BATTERY_MAX_V ) / 2;
}

