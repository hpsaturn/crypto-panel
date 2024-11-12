#include <Preferences.h>

Preferences cfg;
const char* app_name = "crypto_currency";
const char* key_boot_count = "key_boot_count";
const char* key_panel_temp = "key_panel_temp";
const char* key_sleep_time = "key_sleep_time";
const char* key_cur_base = "key_cur_base";
const char* key_ntp_server = "kntpserver";
const char* key_tzone = "ktzone";

// change these params via CLI:
const char * default_server = "pool.ntp.org";  
const char * default_tzone = "CET-1CEST,M3.5.0,M10.5.0/3";

String getKeyName(int cryptoId) {
  char key[15];
  sprintf(key, "key_crypto%02d", cryptoId);
  return String(key);
}

void listCryptos(bool load = false) {
  cfg.begin(app_name, RO_MODE);
  int cryptoId = 1;
  if(!load) Serial.println("\nSaved Cryptocurrencies:\n");
  while (cfg.isKey(getKeyName(cryptoId).c_str())) {
    String key = getKeyName(cryptoId);
    String crypto = cfg.getString(key.c_str(), "");
    if (!load) Serial.printf("%d: [%s]\r\n", cryptoId, crypto.c_str());
    if (load) cryptos[cryptoId - 1].apiName = crypto;
    cryptosCount = cryptoId;
    cryptoId++;
  }
  Serial.println("");
  cfg.end();
}

bool saveCrypto(String crypto) {
  if (crypto.length() == 0) {
    Serial.println("\nInvalid cryptocurrency name");
    return false;
  }
  cfg.begin(app_name, RW_MODE);
  int cryptoId = cfg.getInt("crypto_count", 0);
  if (cryptoId == 3) {
    Serial.println("\nMaximum number of cryptos reached.");
    cfg.end();
    return false;
  }
  String key = getKeyName(cryptoId + 1);
  Serial.printf("Saving cryptocurrency: [%s][%s]\r\n", key.c_str(), crypto.c_str());
  cfg.putString(key.c_str(), crypto);
  cfg.putInt("crypto_count", cryptoId + 1);
  cfg.end();
  cryptos[cryptoId].apiName = crypto;
  cryptosCount = cryptoId + 1;
  return true;
}

bool deleteCrypto(String crypto) {
  if (crypto.length() == 0) {
    Serial.println("\nerror: crypto is empty, please pass a valid crypto name");
    return false;
  }
  int cryptoId = 1;
  bool dropped = false;
  cfg.begin(app_name, RW_MODE);
  while (cfg.isKey(getKeyName(cryptoId).c_str())) {
    String key = getKeyName(cryptoId++);
    String crypto_ = cfg.getString(key.c_str(), "");
    if (!dropped && crypto_.equals(crypto)) {
      Serial.printf("\nDeleting crypto [%s][%s]\r\n", key.c_str(), crypto.c_str());
      cfg.remove(key.c_str());
      dropped = true;
      int cryptoId_count = cfg.getInt("crypto_count", 0);
      cfg.putInt("crypto_count", cryptoId_count - 1);
      continue;
    }
    if (dropped) {
      String crypto_drop = cfg.getString(key.c_str(), "");
      String key_drop = getKeyName(cryptoId - 2);
      cfg.putString(key_drop.c_str(), crypto_drop);
      cfg.remove(key.c_str());
    }
  }
  cfg.end();
  if (!dropped) {
    Serial.println("\nerror: crypto not found");
    return false;
  }
  return dropped;
}

void setInt(String key, int value) {
  cfg.begin(app_name, RW_MODE);
  cfg.putInt(key.c_str(), value);
  cfg.end();
}

int32_t getInt(String key, int defaultValue) {
  cfg.begin(app_name, RO_MODE);
  int32_t out = cfg.getInt(key.c_str(), defaultValue);
  cfg.end();
  return out;
}

void setString(String key, String value) {
  cfg.begin(app_name, RW_MODE);
  cfg.putString(key.c_str(), value);
  cfg.end();
}

String getString(String key, String defaultValue) {
  cfg.begin(app_name, RO_MODE);
  String out = cfg.getString(key.c_str(), defaultValue);
  cfg.end();
  return out;
}

void clearSettings(){
  cfg.begin(app_name, RW_MODE);
  cfg.clear();
  cfg.end();
}