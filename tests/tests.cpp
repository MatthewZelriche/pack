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

      // Test uint16 (3 bytes)
      uint16_t val4 = 256;
      uint32_t val5 = 30000;
      packer.Serialize(val4, val5);
      REQUIRE(packer.ByteCount() == 15);

      // Test uint32 (5 bytes)
      uint32_t val6 = 70000;
      uint64_t val7 = 1234567;
      packer.Serialize(val6, val7);
      REQUIRE(packer.ByteCount() == 25);

      // Test uint64 (9 bytes)
      uint64_t val8 = UINT64_MAX;
      packer.Serialize(val8);
      REQUIRE(packer.ByteCount() == 34);
   }

   {
      pack::Unpacker unpacker(stream);
      uint8_t val1;
      uint8_t val2;
      uint16_t val3;
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

      uint8_t narrow16;
      REQUIRE_THROWS_AS(unpacker.Deserialize(narrow16), std::length_error);
      uint16_t val4;
      uint16_t val5;
      unpacker.Deserialize(val4, val5);
      REQUIRE(val4 == 256);
      REQUIRE(val5 == 30000);
      REQUIRE(unpacker.ByteCount() == 15);

      uint16_t narrow32;
      REQUIRE_THROWS_AS(unpacker.Deserialize(narrow32), std::length_error);
      uint32_t val6;
      uint64_t val7;
      unpacker.Deserialize(val6, val7);
      REQUIRE(val6 == 70000);
      REQUIRE(val7 == 1234567);
      REQUIRE(unpacker.ByteCount() == 25);

      uint32_t narrow64;
      REQUIRE_THROWS_AS(unpacker.Deserialize(narrow64), std::length_error);
      uint64_t val8;
      unpacker.Deserialize(val8);
      REQUIRE(val8 == UINT64_MAX);
      REQUIRE(unpacker.ByteCount() == 34);

      uint64_t invalid;
      REQUIRE_THROWS_AS(unpacker.Deserialize(invalid), std::invalid_argument);
   }

   stream.str("");
   {
      pack::Unpacker unpacker(stream);
      bool invalid;
      REQUIRE_THROWS_AS(unpacker.Deserialize(invalid), std::invalid_argument);
   }

   stream.put(0xd2);
   {
      pack::Unpacker unpacker(stream);
      uint32_t invalid;
      REQUIRE_THROWS_AS(unpacker.Deserialize(invalid), std::runtime_error);
   }
}