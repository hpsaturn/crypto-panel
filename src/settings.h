Preferences preferences;
const char* app_name = "crypto_currency";
const char* key_boot_count = "key_boot_count";

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