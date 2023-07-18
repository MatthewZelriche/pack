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
      uint32_t invalid;
      REQUIRE_THROWS_AS(unpacker.Deserialize(invalid), std::invalid_argument);
   }

   stream.put(0xd2);
   {
      pack::Unpacker unpacker(stream);
      uint32_t invalid;
      REQUIRE_THROWS_AS(unpacker.Deserialize(invalid), std::runtime_error);
   }
}

TEST_CASE("Signed Integer") {
   std::stringstream stream(std::ios::binary | std::ios::out | std::ios::in);
   {
      pack::Packer packer(stream);

      // Test fixints (1 byte)
      int8_t fixint1 = -1;
      int16_t fixint2 = -12;
      int32_t fixint3 = -32;
      packer.Serialize(fixint1, fixint2, fixint3);
      REQUIRE(packer.ByteCount() == 3);

      // Test int8 (2 bytes)
      int16_t val1 = 100;
      int32_t val2 = INT8_MIN;
      int64_t val3 = INT8_MAX;
      packer.Serialize(val1, val2, val3);
      REQUIRE(packer.ByteCount() == 7);

      // Test int16 (3 bytes)
      int16_t val4 = -32001;
      int32_t val5 = 29487;
      packer.Serialize(val4, val5);
      REQUIRE(packer.ByteCount() == 13);

      // Test int32 (5 bytes)
      int32_t val6 = INT32_MAX;
      int64_t val7 = INT32_MIN;
      packer.Serialize(val6, val7);
      REQUIRE(packer.ByteCount() == 23);

      // Test int64 (9 bytes)
      int64_t val8 = INT64_MIN;
      packer.Serialize(val8);
      REQUIRE(packer.ByteCount() == 32);
   }

   {
      pack::Unpacker unpacker(stream);
      int8_t val1;
      int8_t val2;
      int16_t val3;
      unpacker.Deserialize(val1, val2, val3);
      REQUIRE(val1 == -1);
      REQUIRE(val2 == -12);
      REQUIRE(val3 == -32);
      REQUIRE(unpacker.ByteCount() == 3);

      unpacker.Deserialize(val1, val2, val3);
      REQUIRE(val1 == 100);
      REQUIRE(val2 == INT8_MIN);
      REQUIRE(val3 == INT8_MAX);
      REQUIRE(unpacker.ByteCount() == 7);

      int8_t narrow16;
      REQUIRE_THROWS_AS(unpacker.Deserialize(narrow16), std::length_error);
      int16_t val4;
      int16_t val5;
      unpacker.Deserialize(val4, val5);
      REQUIRE(val4 == -32001);
      REQUIRE(val5 == 29487);
      REQUIRE(unpacker.ByteCount() == 13);

      int16_t narrow32;
      REQUIRE_THROWS_AS(unpacker.Deserialize(narrow32), std::length_error);
      int32_t val6;
      int64_t val7;
      unpacker.Deserialize(val6, val7);
      REQUIRE(val6 == INT32_MAX);
      REQUIRE(val7 == INT32_MIN);
      REQUIRE(unpacker.ByteCount() == 23);

      int32_t narrow64;
      REQUIRE_THROWS_AS(unpacker.Deserialize(narrow64), std::length_error);
      int64_t val8;
      unpacker.Deserialize(val8);
      REQUIRE(val8 == INT64_MIN);
      REQUIRE(unpacker.ByteCount() == 32);

      int64_t invalid;
      REQUIRE_THROWS_AS(unpacker.Deserialize(invalid), std::invalid_argument);
   }

   stream.str("");
   {
      pack::Unpacker unpacker(stream);
      int32_t invalid;
      REQUIRE_THROWS_AS(unpacker.Deserialize(invalid), std::invalid_argument);
   }

   stream.put(0xca);
   {
      pack::Unpacker unpacker(stream);
      int32_t invalid;
      REQUIRE_THROWS_AS(unpacker.Deserialize(invalid), std::runtime_error);
   }
}

std::string StringOfSize(size_t len) {
   std::string str;

   for (int i = 0; i < len; i++) { str.append(1, (char)('a' + (rand() % 26))); }
   return str;
}

TEST_CASE("String") {
   srand(time(NULL));
   std::stringstream stream(std::ios::binary | std::ios::out | std::ios::in);
   std::string three = StringOfSize(3);
   std::string thirtyone = StringOfSize(31);
   std::string fortytwo = StringOfSize(42);
   std::string uint8max = StringOfSize(UINT8_MAX);
   std::string str16 = StringOfSize(UINT8_MAX * 5);
   std::string str16max = StringOfSize(UINT16_MAX);
   std::string large = StringOfSize(100000);
   {
      pack::Packer packer(stream);
      // Test fixstr
      char arr[12] = {0};
      memcpy(arr, three.data(), 3);
      packer.Serialize(arr, thirtyone);
      REQUIRE(packer.ByteCount() == 36);

      // Test STR8
      char arr2[43] = {0};
      memcpy(arr2, fortytwo.data(), 42);
      packer.Serialize(arr2, uint8max);
      REQUIRE(packer.ByteCount() == 337);

      // Test STR16
      char arr3[UINT8_MAX * 5 + 1] = {0};
      memcpy(arr3, str16.data(), UINT8_MAX * 5);
      packer.Serialize(arr3, str16max);
      REQUIRE(packer.ByteCount() == 67153);

      // Test STR32
      packer.Serialize(large);
      REQUIRE(packer.ByteCount() == 167158);
   }

   {
      pack::Unpacker unpacker(stream);
      char tooShort[3] = {0};
      REQUIRE_THROWS_AS(unpacker.Deserialize(tooShort), std::length_error);
      char arr[12] = {0};
      std::string string;
      unpacker.Deserialize(arr, string);
      REQUIRE(std::strcmp(arr, three.c_str()) == 0);
      REQUIRE(std::strcmp(string.c_str(), thirtyone.c_str()) == 0);
      REQUIRE(unpacker.ByteCount() == 36);

      char arr2[43] = {0};
      std::string string3;
      unpacker.Deserialize(arr2, string3);
      REQUIRE(std::strcmp(arr2, fortytwo.c_str()) == 0);
      REQUIRE(std::strcmp(string3.c_str(), uint8max.c_str()) == 0);
      REQUIRE(unpacker.ByteCount() == 337);

      char arr3[UINT8_MAX * 5 + 1] = {0};
      std::string string4;
      unpacker.Deserialize(arr3, string4);
      REQUIRE(std::strcmp(arr3, str16.c_str()) == 0);
      REQUIRE(std::strcmp(string4.c_str(), str16max.c_str()) == 0);
      REQUIRE(unpacker.ByteCount() == 67153);

      std::string string5;
      unpacker.Deserialize(string5);
      REQUIRE(std::strcmp(string5.c_str(), large.c_str()) == 0);
      REQUIRE(unpacker.ByteCount() == 167158);
   }
}

TEST_CASE("Floating Point") {
   std::stringstream stream(std::ios::binary | std::ios::out | std::ios::in);
   float pi = 3.14159f;
   float zero = 0.0f;
   float max = FLT_MAX;
   float infinity = std::numeric_limits<float>::infinity();
   double min = DBL_MIN;
   double sq2 = 1.14;
   {
      pack::Packer packer(stream);
      packer.Serialize(pi, zero, max, infinity, min, sq2);
      REQUIRE(packer.ByteCount() == 38);
   }

   {
      pack::Unpacker unpacker(stream);
      float one;
      float two;
      float three;
      float four;
      double five;
      double six;
      unpacker.Deserialize(one, two, three, four, five, six);
      REQUIRE(std::abs(one - pi) < std::numeric_limits<float>::epsilon());
      REQUIRE(std::abs(two - zero) < std::numeric_limits<float>::epsilon());
      REQUIRE(std::abs(three - max) < std::numeric_limits<float>::epsilon());
      REQUIRE(four == std::numeric_limits<float>::infinity());
      REQUIRE(std::abs(five + min) < std::numeric_limits<double>::epsilon());
      REQUIRE(std::abs(six - sq2) < std::numeric_limits<double>::epsilon());
   }
}

TEST_CASE("Arrays") {
   std::stringstream stream(std::ios::binary | std::ios::out | std::ios::in);

   int arr1_in[4] = {5, 4, 3, 2};
   int arr2_in[] = {16, 15, 14, 13, 12, 11, 10, -1, -2, -3, -4, -5, -6, -7, -8, -9};
   constexpr int len = UINT16_MAX + 20;
   int arr3_in[len] = {0};
   for (int i = 0; i < len; i++) { arr3_in[i] = i; }
   std::array<int, 5> arr4_in = {3, -99999, 9, 0, 42};
   std::vector<int> arr5_in = {-9142, -9143, -9144, -9145, -9146};

   {
      pack::Packer packer(stream);

      packer.Serialize(arr1_in, arr2_in);
      REQUIRE(packer.ByteCount() == 24);
      packer.Serialize(arr3_in);
      packer.Serialize(arr4_in);
      packer.Serialize(arr5_in);
   }

   {
      pack::Unpacker unpacker(stream);
      int arr1[4];
      int arr2[16];
      int arr3[len];
      std::array<int, 5> arr4;

      int tooSmall[3];
      REQUIRE_THROWS_AS(unpacker.Deserialize(tooSmall), std::length_error);
      unpacker.Deserialize(arr1);
      REQUIRE(std::memcmp((void *)arr1_in, (void *)arr1, 4 * sizeof(int)) == 0);

      int tooSmall2[14];
      REQUIRE_THROWS_AS(unpacker.Deserialize(tooSmall2), std::length_error);
      unpacker.Deserialize(arr2);
      REQUIRE(unpacker.ByteCount() == 24);
      REQUIRE(std::memcmp((void *)arr2_in, (void *)arr2, 16 * sizeof(int)) == 0);

      unpacker.Deserialize(arr3, arr4);
      REQUIRE(std::memcmp((void *)arr3_in, (void *)arr3, len * sizeof(int)) == 0);
      REQUIRE(arr4 == arr4_in);

      std::vector<int> arr5;
      unpacker.Deserialize(arr5);
      REQUIRE(arr5 == arr5_in);
   }
}