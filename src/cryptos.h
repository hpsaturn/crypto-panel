struct Price {
  double inr;
  String btc;
  String eth;
};

struct Crypto
{
  String apiName;
  String symbol;
  Price price;
  double dayChange;
  double weekChange;
};

struct News
{
  String author;
  String title;
  String summary;
  String published;
  String link;
  uint32_t qrsize;
  const char* qr;
};

// ----------------------------
// Coin id list - adjust it to meet your interests
//
// Put your cryptocurrencies in the array below.
// Get id of your coin here: https://api.coingecko.com/api/v3/coins/list?include_platform=false
// ----------------------------

Crypto cryptos[] = {
    {"bitcoin"},
    {"ethereum"},
    {"cardano"},
 };

int cryptosCount = (sizeof(cryptos) / sizeof(cryptos[0]));

News news;