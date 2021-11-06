#define BATT_PIN 36
#define SD_MISO 12
#define SD_MOSI 13
#define SD_SCLK 14
#define SD_CS 15


bool devmod = (bool)CORE_DEBUG_LEVEL; // extra debug msgs

void logMemory() {
  if(devmod) Serial.printf("-->[IHAL] Used PSRAM: %d\n", ESP.getPsramSize() - ESP.getFreePsram());
}
