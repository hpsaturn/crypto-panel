#define BATT_PIN 36
#define SD_MISO 12
#define SD_MOSI 13
#define SD_SCLK 14
#define SD_CS 15

void logMemory() {
  Serial.printf("-->[IHAL] Used PSRAM: %d\n", ESP.getPsramSize() - ESP.getFreePsram());
}
