#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <pack/msgpack.hpp>

TEST_CASE("Boolean") {
   std::stringstream stream(std::ios::binary | std::ios::out | std::ios::in);
   {
      pack::Packer packer(stream);
      packer.Serialize(true, false);
      REQUIRE(packer.ByteCount() == 2);
   }

   {
      pack::Unpacker unpacker(stream);
      bool first;
      bool second;
      unpacker.Deserialize(first, second);
      REQUIRE(first == true);
      REQUIRE(second == false);
      REQUIRE(unpacker.ByteCount() == 2);
      bool invalid;
      REQUIRE_THROWS_AS(unpacker.Deserialize(invalid), std::invalid_argument);
   }

   stream.str("");
   {
      pack::Unpacker unpacker(stream);
      bool invalid;
      REQUIRE_THROWS_AS(unpacker.Deserialize(invalid), std::invalid_argument);
   }

   stream.put(0xcc);
   {
      pack::Unpacker unpacker(stream);
      bool invalid;
      REQUIRE_THROWS_AS(unpacker.Deserialize(invalid), std::runtime_error);
   }
}

TEST_CASE("Unsigned Integer") {
   std::stringstream stream(std::ios::binary | std::ios::out | std::ios::in);
   {
      pack::Packer packer(stream);

      // Test fixints (1 byte)
      uint8_t fixint1 = 0;
      uint16_t fixint2 = 35;
      uint32_t fixint3 = 127;
      packer.Serialize(fixint1, fixint2, fixint3);
      REQUIRE(packer.ByteCount() == 3);

      // Test uint8 (2 bytes)
      uint16_t val1 = 128;
      uint32_t val2 = 180;
      uint64_t val3 = 255;
      packer.Serialize(val1, val2, val3);
      REQUIRE(packer.ByteCount() == 9);
   }

   {
      pack::Unpacker unpacker(stream);
      uint8_t val1;
      uint8_t val2;
      uint8_t val3;
      unpacker.Deserialize(val1, val2, val3);
      REQUIRE(val1 == 0);
      REQUIRE(val2 == 35);
      REQUIRE(val3 == 127);
      REQUIRE(unpacker.ByteCount() == 3);

      unpacker.Deserialize(val1, val2, val3);
      REQUIRE(val1 == 128);
      REQUIRE(val2 == 180);
      REQUIRE(val3 == 255);
      REQUIRE(unpacker.ByteCount() == 9);
   }
}