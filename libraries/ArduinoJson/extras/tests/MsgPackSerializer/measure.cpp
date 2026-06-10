// ArduinoJson - https://arduinojson.org
// Copyright © 2014-2026, Benoit BLANCHON
// MIT License

#include <ArduinoJson.h>
#include <catch.hpp>

TEST_CASE("measureMsgPack()") {
  DynamicJsonDocument doc(4096);
  JsonObject object = doc.to<JsonObject>();
  object["hello"] = "world";

  REQUIRE(measureMsgPack(doc) == 13);
}
