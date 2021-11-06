// ----------------------------
// Functions used to download data from coingecko retrieve are separated in this file
// ----------------------------

const char* rootCACertificate =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDdzCCAl+gAwIBAgIEAgAAuTANBgkqhkiG9w0BAQUFADBaMQswCQYDVQQGEwJJ\n"
    "RTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJlclRydXN0MSIwIAYD\n"
    "VQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTAwMDUxMjE4NDYwMFoX\n"
    "DTI1MDUxMjIzNTkwMFowWjELMAkGA1UEBhMCSUUxEjAQBgNVBAoTCUJhbHRpbW9y\n"
    "ZTETMBEGA1UECxMKQ3liZXJUcnVzdDEiMCAGA1UEAxMZQmFsdGltb3JlIEN5YmVy\n"
    "VHJ1c3QgUm9vdDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKMEuyKr\n"
    "mD1X6CZymrV51Cni4eiVgLGw41uOKymaZN+hXe2wCQVt2yguzmKiYv60iNoS6zjr\n"
    "IZ3AQSsBUnuId9Mcj8e6uYi1agnnc+gRQKfRzMpijS3ljwumUNKoUMMo6vWrJYeK\n"
    "mpYcqWe4PwzV9/lSEy/CG9VwcPCPwBLKBsua4dnKM3p31vjsufFoREJIE9LAwqSu\n"
    "XmD+tqYF/LTdB1kC1FkYmGP1pWPgkAx9XbIGevOF6uvUA65ehD5f/xXtabz5OTZy\n"
    "dc93Uk3zyZAsuT3lySNTPx8kmCFcB5kpvcY67Oduhjprl3RjM71oGDHweI12v/ye\n"
    "jl0qhqdNkNwnGjkCAwEAAaNFMEMwHQYDVR0OBBYEFOWdWTCCR1jMrPoIVDaGezq1\n"
    "BE3wMBIGA1UdEwEB/wQIMAYBAf8CAQMwDgYDVR0PAQH/BAQDAgEGMA0GCSqGSIb3\n"
    "DQEBBQUAA4IBAQCFDF2O5G9RaEIFoN27TyclhAO992T9Ldcw46QQF+vaKSm2eT92\n"
    "9hkTI7gQCvlYpNRhcL0EYWoSihfVCr3FvDB81ukMJY2GQE/szKN+OMY3EU/t3Wgx\n"
    "jkzSswF07r51XgdIGn9w/xZchMB5hbgF/X++ZRGjD8ACtPhSNzkE1akxehi/oCr0\n"
    "Epn3o0WC4zxe9Z2etciefC7IpJ5OCBRLbf1wbWsaY71k5h+3zvDyny67G7fyUIhz\n"
    "ksLi4xaNmjICq44Y3ekQEe5+NauQrz4wlHrQMz2nZQ/1/I6eYs9HRCwBXbsdtTLS\n"
    "R9I4LtD+gdwyah617jzV/OeBHRnDJELqYzmp\n"
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
    //client.setFingerprint(coingeckoSslFingerprint);
    http.useHTTP10(true);
    client.setCACert(rootCACertificate);

    String apiUrl = "https://api.coingecko.com/api/v3/simple/price?ids=" + combineCryptoCurrencies() + "&vs_currencies=btc%2Ceth";

    // Serial.println("-->[cAPI] target: "+apiUrl);

    client.connect("api.coingecko.com", 443);
    http.begin(client, apiUrl);

    int code = http.GET();
    if (code != HTTP_CODE_OK) {
        Serial.printf("-->[cAPI] Error connecting to API while downloading BTC and ETH data. code: %i\n",code);
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
    //client.setFingerprint(coingeckoSslFingerprint);

    String apiUrl = "https://api.coingecko.com/api/v3/coins/markets?vs_currency=" + vsCurrency + "&ids=" + combineCryptoCurrencies() + "&order=market_cap_desc&per_page=100&page=1&sparkline=false&price_change_percentage=24h%2C7d";
    
    // Serial.println("-->[cAPI] target: "+apiUrl);

    client.connect("api.coingecko.com", 443);

    http.begin(client, apiUrl);

    int code = http.GET();
    if (code != HTTP_CODE_OK) {
        Serial.printf("-->[cAPI] Error connecting to API while downloading base data. code: %i\n",code);
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

    String apiUrl = "http://influxdb.canair.io:8080/posts";
    
    Serial.println("-->[cAPI] target news: "+apiUrl);

    http.begin(client, apiUrl);

    int code = http.GET();
    if (code != HTTP_CODE_OK) {
        Serial.printf("-->[nAPI] Error connecting to API while downloading news data. code: %i\n",code);
        stopClient();
        return false;
    }

    SpiRamJsonDocument doc(128000);
    DeserializationError error = deserializeJson(doc,http.getStream());
    

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

