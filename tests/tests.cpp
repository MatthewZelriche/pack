#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <pack/msgpack.hpp>

TEST_CASE("Boolean") {
   pack::ByteArray data;

   data = pack::Serialize(true);
   REQUIRE(pack::Deserialize<bool>(data) == true);

   data = pack::Serialize(false);
   REQUIRE(pack::Deserialize<bool>(data) == false);

   data.clear();
   REQUIRE_THROWS_AS(pack::Deserialize<bool>(data), std::invalid_argument);

   data.clear();
   data.push_back(0xc0);
   REQUIRE_THROWS_AS(pack::Deserialize<bool>(data), std::runtime_error);
}