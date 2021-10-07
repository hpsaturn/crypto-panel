
#include <shoddyxml.h>

const struct site_t {
  char *title;
  char *url;
  char *contentsToDisplay;
} sites[] = {
  // {"CNN.com", "http://rss.cnn.com/rss/edition.rss", "title"}
  {"Cointelegraph", "https://cointelegraph.com/rss/tag/bitcoin","description"}
  // {"BBC News", "http://feeds.bbci.co.uk/news/rss.xml", "description"},
};

const int delayPerRSS = 1000;

int itemDepth = 0;
int lastTagMatches = 0;
char *contentsToDisplay;

String rssout = "";

int httpGetChar();

WiFiClient *stream;
shoddyxml x(httpGetChar);

void foundXMLDeclOrEnd() {

}

void foundPI(char *s) {

}

void foundSTag(char *s, int numAttributes, attribute_t attributes[]) {
  if (strcmp(s, "item") == 0) {
    itemDepth++;
  }

  if (strcmp(s, contentsToDisplay) == 0) {
    lastTagMatches = 1;
  } else {
    lastTagMatches = 0;
  }
}

void foundETag(char *s) {
  if ((itemDepth == 1) && (strcmp(s, contentsToDisplay) == 0)) {
    Serial.println("");
  }
  if (strcmp(s, "item") == 0) {
    itemDepth--;
  }
}

void foundEmptyElemTag(char *s, int numAttributes, attribute_t attributes[]) {

}

void foundCharacter(char c) {
  if ((itemDepth == 1) && (lastTagMatches == 1)) {
    Serial.print(c);
    rssout = rssout + String(c); 
  }
}

void foundElement(char *s) {

}

int httpGetChar() {
  if (http.connected()) {
    if (stream->available()) {
      return stream->read();
    } else {
      return 0;
    }
  }
  return EOF;
}

void xmlInit() {
  x.foundXMLDecl = foundXMLDeclOrEnd;
  x.foundXMLEnd = foundXMLDeclOrEnd;
  x.foundPI = foundPI;
  x.foundSTag = foundSTag;
  x.foundETag = foundETag;
  x.foundEmptyElemTag = foundEmptyElemTag;
  x.foundCharacter = foundCharacter;
  x.foundElement = foundElement;
}

void xmlLoop() {
  for (int i = 0; i < sizeof(sites) / sizeof(struct site_t); i++) {
      itemDepth = 0;
      lastTagMatches = 0;

      Serial.println(sites[i].title);
      contentsToDisplay = const_cast<char*>(sites[i].contentsToDisplay);
      http.begin(sites[i].url);
      int httpCode = http.GET();
      if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
          stream = http.getStreamPtr();
          x.parse();
        }
      }
      http.end();
      delay(delayPerRSS);
  }
}

