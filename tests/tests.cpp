#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <pack/msgpack.hpp>

TEST_CASE("Boolean") {
   std::stringstream stream(std::ios::binary | std::ios::out | std::ios::in);
   {
      pack::Packer packer(stream);
      REQUIRE(packer.Serialize(true) == true);
      REQUIRE(packer.Serialize(false) == true);
      REQUIRE(packer.ByteCount() == 2);
   }

   {
      pack::Unpacker unpacker(stream);
      REQUIRE(unpacker.Deserialize<bool>() == true);
      REQUIRE(unpacker.Deserialize<bool>() == false);
      REQUIRE(unpacker.ByteCount() == 2);
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

TEST_CASE("Unsigned Integer") {
   std::stringstream stream(std::ios::binary | std::ios::out | std::ios::in);
   {
      pack::Packer packer(stream);

      // Test fixints (1 byte)
      uint8_t fixint1 = 0;
      uint16_t fixint2 = 35;
      uint32_t fixint3 = 127;
      REQUIRE(packer.Serialize(fixint1) == true);
      REQUIRE(packer.Serialize(fixint2) == true);
      REQUIRE(packer.Serialize(fixint3) == true);
      REQUIRE(packer.ByteCount() == 3);

      // Test uint8 (2 bytes)
      uint16_t val1 = 128;
      uint32_t val2 = 180;
      uint64_t val3 = 255;
      REQUIRE(packer.Serialize(val1) == true);
      REQUIRE(packer.Serialize(val2) == true);
      REQUIRE(packer.Serialize(val3) == true);
      REQUIRE(packer.ByteCount() == 9);
   }

   {
      pack::Unpacker unpacker(stream);
      REQUIRE(unpacker.Deserialize<uint8_t>() == 0);
      REQUIRE(unpacker.Deserialize<uint32_t>() == 35);
      REQUIRE(unpacker.Deserialize<uint16_t>() == 127);
      REQUIRE(unpacker.ByteCount() == 3);

      REQUIRE(unpacker.Deserialize<uint8_t>() == 128);
      REQUIRE(unpacker.Deserialize<uint8_t>() == 180);
      REQUIRE(unpacker.Deserialize<uint8_t>() == 255);
      REQUIRE(unpacker.ByteCount() == 9);
   }
}