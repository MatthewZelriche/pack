#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <pack/msgpack.hpp>

TEST_CASE("Boolean") {
   std::stringstream stream(std::ios::binary | std::ios::out | std::ios::in);
   {
      pack::Packer packer(stream);
      REQUIRE(packer.Serialize(true) == true);
      REQUIRE(packer.Serialize(false) == true);
   }

   {
      pack::Unpacker unpacker(stream);
      REQUIRE(unpacker.Deserialize<bool>() == true);
      REQUIRE(unpacker.Deserialize<bool>() == false);
      REQUIRE_THROWS_AS(unpacker.Deserialize<bool>(), std::invalid_argument);
   }

   stream.str("");
   {
      pack::Unpacker unpacker(stream);
      REQUIRE_THROWS_AS(unpacker.Deserialize<bool>(), std::invalid_argument);
   }

   stream.put(0xcc);
   {
      pack::Unpacker unpacker(stream);
      REQUIRE_THROWS_AS(unpacker.Deserialize<bool>(), std::runtime_error);
   }
}