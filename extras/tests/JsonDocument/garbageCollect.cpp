// ArduinoJson - https://arduinojson.org
// Copyright © 2014-2023, Benoit BLANCHON
// MIT License

#include <ArduinoJson.h>
#include <stdlib.h>  // malloc, free
#include <catch.hpp>
#include <utility>

#include "Allocators.hpp"

using ArduinoJson::detail::sizeofObject;
using ArduinoJson::detail::sizeofString;

TEST_CASE("JsonDocument::garbageCollect()") {
  ControllableAllocator controllableAllocator;
  SpyingAllocator spyingAllocator(&controllableAllocator);
  JsonDocument doc(4096, &spyingAllocator);

  SECTION("when allocation succeeds") {
    deserializeJson(doc, "{\"blanket\":1,\"dancing\":2}");
    REQUIRE(doc.memoryUsage() == sizeofObject(2) + 2 * sizeofString(7));
    doc.remove("blanket");
    spyingAllocator.clearLog();

    bool result = doc.garbageCollect();

    REQUIRE(result == true);
    REQUIRE(doc.memoryUsage() == sizeofObject(1) + sizeofString(7));
    REQUIRE(doc.as<std::string>() == "{\"dancing\":2}");
    REQUIRE(spyingAllocator.log() ==
            AllocatorLog() << AllocatorLog::Allocate(4096)
                           << AllocatorLog::Allocate(sizeofString(7))
                           << AllocatorLog::Deallocate(sizeofString(7))
                           << AllocatorLog::Deallocate(sizeofString(7))
                           << AllocatorLog::Deallocate(4096));
  }

  SECTION("when allocation fails") {
    deserializeJson(doc, "{\"blanket\":1,\"dancing\":2}");
    REQUIRE(doc.memoryUsage() == sizeofObject(2) + 2 * sizeofString(7));
    doc.remove("blanket");
    controllableAllocator.disable();
    spyingAllocator.clearLog();

    bool result = doc.garbageCollect();

    REQUIRE(result == false);
    REQUIRE(doc.memoryUsage() == sizeofObject(2) + 2 * sizeofString(7));
    REQUIRE(doc.as<std::string>() == "{\"dancing\":2}");

    REQUIRE(spyingAllocator.log() == AllocatorLog()
                                         << AllocatorLog::AllocateFail(4096));
  }
}
