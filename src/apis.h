// ----------------------------
// Functions used to download data from coingecko retrieve are separated in this file
// ----------------------------

const char* rootCACertificate =
    "-----BEGIN CERTIFICATE-----\n"
    "MIICxjCCAk2gAwIBAgIRALO93/inhFu86QOgQTWzSkUwCgYIKoZIzj0EAwMwTzEL\n"
    "MAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2VhcmNo\n"
    "IEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDIwHhcNMjAwOTA0MDAwMDAwWhcN\n"
    "MjUwOTE1MTYwMDAwWjAyMQswCQYDVQQGEwJVUzEWMBQGA1UEChMNTGV0J3MgRW5j\n"
    "cnlwdDELMAkGA1UEAxMCRTEwdjAQBgcqhkjOPQIBBgUrgQQAIgNiAAQkXC2iKv0c\n"
    "S6Zdl3MnMayyoGli72XoprDwrEuf/xwLcA/TmC9N/A8AmzfwdAVXMpcuBe8qQyWj\n"
    "+240JxP2T35p0wKZXuskR5LBJJvmsSGPwSSB/GjMH2m6WPUZIvd0xhajggEIMIIB\n"
    "BDAOBgNVHQ8BAf8EBAMCAYYwHQYDVR0lBBYwFAYIKwYBBQUHAwIGCCsGAQUFBwMB\n"
    "MBIGA1UdEwEB/wQIMAYBAf8CAQAwHQYDVR0OBBYEFFrz7Sv8NsI3eblSMOpUb89V\n"
    "yy6sMB8GA1UdIwQYMBaAFHxClq7eS0g7+pL4nozPbYupcjeVMDIGCCsGAQUFBwEB\n"
    "BCYwJDAiBggrBgEFBQcwAoYWaHR0cDovL3gyLmkubGVuY3Iub3JnLzAnBgNVHR8E\n"
    "IDAeMBygGqAYhhZodHRwOi8veDIuYy5sZW5jci5vcmcvMCIGA1UdIAQbMBkwCAYG\n"
    "Z4EMAQIBMA0GCysGAQQBgt8TAQEBMAoGCCqGSM49BAMDA2cAMGQCMHt01VITjWH+\n"
    "Dbo/AwCd89eYhNlXLr3pD5xcSAQh8suzYHKOl9YST8pE9kLJ03uGqQIwWrGxtO3q\n"
    "YJkgsTgDyj2gJrjubi1K9sZmHzOa25JK1fUpE8ZwYii6I4zPPS/Lgul/\n"
    "-----END CERTIFICATE-----";

HTTPClient http;
WiFiClientSecure client;

const char* coingeckoSslFingerprint = "8925605d5044fcc0852b98d7d3665228684de6e2";

struct SpiRamAllocator {
  void* allocate(size_t size) {
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
  }

  void deallocate(void* pointer) {
    heap_caps_free(pointer);
  }

  void* reallocate(void* ptr, size_t new_size) {
    return heap_caps_realloc(ptr, new_size, MALLOC_CAP_SPIRAM);
  }
};

using SpiRamJsonDocument = BasicJsonDocument<SpiRamAllocator>;

String combineCryptoCurrencies() {
  String cryptosString = "";

  for (int i = 0; i < cryptosCount; i++) {
    cryptosString += cryptos[i].apiName;

    if (i != cryptosCount - 1) {
      cryptosString += "%2C";
    }
  }

  return cryptosString;
}

int getCryptoIndexById(String id) {
  for (int i = 0; i < cryptosCount; i++) {
    if (cryptos[i].apiName == id)
      return i;
  }
  return 0;
}

void stopClient() {
  http.end();
  client.stop();
}

bool downloadBtcAndEthPrice() {
  // client.setFingerprint(coingeckoSslFingerprint);
  http.useHTTP10(true);
  client.setCACert(rootCACertificate);

  String apiUrl = "https://api.coingecko.com/api/v3/simple/price?ids=" + combineCryptoCurrencies() + "&vs_currencies=btc%2Ceth";

  // Serial.println("-->[cAPI] target: "+apiUrl);

  client.connect("api.coingecko.com", 443);
  http.begin(client, apiUrl);

  int code = http.GET();
  if (code != HTTP_CODE_OK) {
    Serial.printf("-->[cAPI] Error connecting to API while downloading BTC and ETH data. code: %i\r\n", code);
    stopClient();
    return false;
  }

  StaticJsonDocument<512> filter;

  for (int i = 0; i < cryptosCount; i++) {
    filter[cryptos[i].apiName]["btc"] = true;
    filter[cryptos[i].apiName]["eth"] = true;
  }

  DynamicJsonDocument doc(4096);
  DeserializationError error = deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));

  if (error) {
    Serial.print(F("-->[cAPI] deserializeJson() failed: "));
    Serial.println(error.f_str());
    stopClient();
    return false;
  }

  Serial.println("-->[cAPI] downloaded crypto data");

  for (int i = 0; i < cryptosCount; i++) {
    JsonObject json = doc[cryptos[i].apiName];
    String btcPrice = json["btc"];
    String ethPrice = json["eth"];

    cryptos[i].price.btc = btcPrice;
    cryptos[i].price.eth = ethPrice;
  }

  stopClient();
  return true;
}

bool downloadBaseData(String vsCurrency) {
  http.useHTTP10(true);
  client.setCACert(rootCACertificate);
  // client.setFingerprint(coingeckoSslFingerprint);

  String apiUrl = "https://api.coingecko.com/api/v3/coins/markets?vs_currency=" + vsCurrency + "&ids=" + combineCryptoCurrencies() + "&order=market_cap_desc&per_page=100&page=1&sparkline=false&price_change_percentage=24h%2C7d";

  // Serial.println("-->[cAPI] target: "+apiUrl);

  client.connect("api.coingecko.com", 443);

  http.begin(client, apiUrl);

  int code = http.GET();
  if (code != HTTP_CODE_OK) {
    Serial.printf("-->[cAPI] Error connecting to API while downloading base data. code: %i\r\n", code);
    stopClient();
    return false;
  }

  StaticJsonDocument<512> filter;

  for (int i = 0; i < cryptosCount; i++) {
    filter[i]["id"] = true;
    filter[i]["symbol"] = true;
    filter[i]["current_price"] = true;
    filter[i]["price_change_percentage_24h_in_currency"] = true;
    filter[i]["price_change_percentage_7d_in_currency"] = true;
  }

  DynamicJsonDocument doc(4096);
  DeserializationError error = deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));

  if (error) {
    Serial.print(F("-->[cAPI] deserializeJson() failed: "));
    Serial.println(error.f_str());
    stopClient();
    return false;
  }

  Serial.println("-->[cAPI] downloaded base data");

  for (int i = 0; i < cryptosCount; i++) {
    JsonObject json = doc[i];
    String id = json["id"];
    int cryptoIndex = getCryptoIndexById(id);

    double currentPrice = json["current_price"];
    cryptos[cryptoIndex].price.inr = currentPrice;

    String symbol = json["symbol"];
    symbol.toUpperCase();
    double dayChange = json["price_change_percentage_24h_in_currency"];
    double weekChange = json["price_change_percentage_7d_in_currency"];

    cryptos[cryptoIndex].symbol = symbol;
    cryptos[cryptoIndex].dayChange = dayChange;
    cryptos[cryptoIndex].weekChange = weekChange;
  }

  stopClient();
  return true;
}

bool downloadNewsData() {
  http.useHTTP10(true);
  WiFiClient client;

  String apiUrl = "http://crypto.hpsaturn.com:8080/posts";

  // Serial.println("-->[cAPI] target news: " + apiUrl);

  http.begin(client, apiUrl);

  int code = http.GET();
  if (code != HTTP_CODE_OK) {
    Serial.printf("-->[nAPI] Error connecting to API while downloading news data. code: %i\r\n", code);
    stopClient();
    return false;
  }

  SpiRamJsonDocument doc(128000);
  DeserializationError error = deserializeJson(doc, http.getStream());

  if (error) {
    Serial.print(F("-->[nAPI] deserializeJson() failed: "));
    Serial.println(error.f_str());
    stopClient();
    return false;
  }

  Serial.println("-->[nAPI] downloaded news data");

  news.title = doc["title"] | "";
  news.author = doc["author"] | "";
  news.summary = doc["summary"] | "";
  news.link = doc["link"] | "";
  news.published = doc["published"] | "";
  news.qrsize = doc["qrsize"].as<uint32_t>() | 0;
  news.qr = doc["qr"];

  stopClient();

  return true;
}
