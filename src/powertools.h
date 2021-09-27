#include <driver/adc.h>
#include "esp_adc_cal.h"
#include "esp_sleep.h"

#define BATTERY_MIN_V 3.3
#define BATTERY_MAX_V 4.2
#define BATTCHARG_MIN_V 4.65
#define BATTCHARG_MAX_V 4.91

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
        log_i("eFuse Vref:%u mV", adc_chars.vref);
        vref = adc_chars.vref;
    }
}

double_t _calcPercentage(double_t volts, float max, float min) {
    double_t percentage = (volts - min) * 100 / (max - min);
    if (percentage > 100) {
        percentage = 100;
    }
    if (percentage < 0) {
        percentage = 0;
    }
    return percentage;
}

bool battIsCharging() {
    return curv > BATTERY_MAX_V + (BATTCHARG_MIN_V - BATTERY_MAX_V ) / 2;
}

double_t battCalcPercentage(double_t volts) {
    if (battIsCharging()){
      return _calcPercentage(volts,BATTCHARG_MAX_V,BATTCHARG_MIN_V);
    } else {
      return _calcPercentage(volts,BATTERY_MAX_V,BATTERY_MIN_V);
    }
}

double_t get_battery_percentage() {
    // When reading the battery voltage, POWER_EN must be turned on
    epd_poweron();
    delay(50);

    uint16_t v = analogRead(BATT_PIN);
    battery_voltage = ((double_t)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
    Serial.println("-->[vADC] " + String(battery_voltage) + "v");

    double_t percent = battCalcPercentage(battery_voltage);

    String voltage = "-->[vADC] Battery charge at " + String(percent) + "%";
    Serial.println(voltage);

    epd_poweroff();
    delay(50);

    return percent;
}

/**
 * Function that return the reason by which ESP32 has been awaken from sleep
 */
String get_wakeup_reason(){
	esp_sleep_wakeup_cause_t wakeup_reason;
	wakeup_reason = esp_sleep_get_wakeup_cause();
    switch(wakeup_reason){
        case ESP_SLEEP_WAKEUP_EXT0 : return "Wakeup caused by external signal using RTC_IO"; break;
        case ESP_SLEEP_WAKEUP_EXT1 : return "Wakeup caused by external signal using RTC_CNTL"; break;
        case ESP_SLEEP_WAKEUP_TIMER : return "Wakeup caused by timer"; break;
        case ESP_SLEEP_WAKEUP_TOUCHPAD : return "Wakeup caused by touchpad"; break;
        case ESP_SLEEP_WAKEUP_ULP : return "Wakeup caused by ULP program"; break;
        default : return "Wakeup was not caused by deep sleep: "+String (wakeup_reason); break;
    }
}

/**
 * Function that returns the reason by which ESP32 has been reseted
 */
String get_reset_reason(int cpu_no) {

  switch (rtc_get_reset_reason(cpu_no)){
    case POWERON_RESET          : return  "Vbat power on reset"; break;
    case SW_RESET               : return  "Software reset digital core"; break;
    case OWDT_RESET             : return  "Legacy watch dog reset digital core"; break;
    case DEEPSLEEP_RESET        : return  "Deep Sleep reset digital core"; break;
    case SDIO_RESET             : return  "Reset by SLC module, reset digital core"; break;
    case TG0WDT_SYS_RESET       : return  "Timer Group0 Watch dog reset digital core"; break;
    case TG1WDT_SYS_RESET       : return  "Timer Group1 Watch dog reset digital core"; break;
    case RTCWDT_SYS_RESET       : return  "RTC Watch dog Reset digital core"; break;
    case INTRUSION_RESET        : return  "Instrusion tested to reset CPU"; break;
    case TGWDT_CPU_RESET        : return  "Time Group reset CPU"; break;
    case SW_CPU_RESET           : return  "Software reset CPU"; break;
    case RTCWDT_CPU_RESET       : return  "RTC Watch dog Reset CPU"; break;
    case EXT_CPU_RESET          : return  "for APP CPU, reseted by PRO CPU"; break;
    case RTCWDT_BROWN_OUT_RESET : return  "Reset when the vdd voltage is not stable"; break;
    case RTCWDT_RTC_RESET       : return  "RTC Watch dog reset digital core and rtc module"; break;
    default : return "Unknown reset reason"; break;
  }
}
