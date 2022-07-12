#include <Preferences.h>

Preferences cfg;
const char* app_name = "crypto_currency";
const char* key_boot_count = "key_boot_count";

String getKeyName(int cryptoId) {
  char key[15];
  sprintf(key, "key_crypto%02d", cryptoId);
  return String(key);
}

void listCryptos() {
  cfg.begin(app_name, RO_MODE);
  int cryptoId = 1;
  Serial.println("\nSaved Crypto currencies:\n");
  while (cfg.isKey(getKeyName(cryptoId).c_str())) {
    String key = getKeyName(cryptoId);
    String crypto = cfg.getString(key.c_str(), "");
    Serial.printf("%d: [%s]\r\n", cryptoId, crypto.c_str());
    cryptoId++;
  }
  cfg.end();
}

bool saveCrypto(String crypto){
  if(crypto.length() == 0) return false;
  cfg.begin(app_name, RW_MODE);
  int cryptoId = cfg.getInt("crypto_count", 0);
  if(cryptoId == 3) {
    Serial.println("\nMaximum number of cryptos reached.");
    cfg.end();
    return false;
  }
  String key = getKeyName(cryptoId + 1);
  Serial.printf("Saving crypto currency: [%s][%s]\r\n",key.c_str(), crypto.c_str());
  cfg.putString(key.c_str(), crypto);
  cfg.putInt("crypto_count", cryptoId + 1);
  cfg.end();
  return true;
}

void setInt(String key, int value){
    cfg.begin(app_name, false);
    cfg.putInt(key.c_str(), value);
    cfg.end();
}

int32_t getInt(String key, int defaultValue){
    cfg.begin(app_name, false);
    int32_t out = cfg.getInt(key.c_str(), defaultValue);
    cfg.end();
    return out;
}