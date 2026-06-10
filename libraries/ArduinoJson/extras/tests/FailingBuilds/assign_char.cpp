// ArduinoJson - https://arduinojson.org
// Copyright © 2014-2026, Benoit BLANCHON
// MIT License

#include <ArduinoJson.h>

// See issue #1498

int main() {
  DynamicJsonDocument doc(1024);
  doc["dummy"] = 'A';
}
