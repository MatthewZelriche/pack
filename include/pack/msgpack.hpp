#pragma once

#include <cstdint>
#include <vector>
#include <stdexcept>

namespace pack {

using Byte = uint8_t;
using ByteArray = std::vector<Byte>;

// clang-format off
enum Formats : Byte {
   POS_FIXINT   = 0b00000000, // 0XXXXXXX   @TODO
   NEG_FIXINT   = 0b11100000, // 111xxxxx   @TODO
   FIXMAP       = 0b10000000, // 1000xxxx   @TODO
   FIXARR       = 0b10010000, // 1001xxxx   @TODO
   FIXSTR       = 0b10100000, // 101xxxxx   @TODO

   NIL          = 0b11000000, // 0xc0       @TODO
   BFALSE       = 0b11000010, // 0xc2
   BTRUE        = 0b11000011, // 0xc3
   BIN8         = 0b11000100, // 0xc4       @TODO
   BIN16        = 0b11000101, // 0xc5       @TODO
   BIN32        = 0b11000110, // 0xc6       @TODO
   EXT8         = 0b11000111, // 0xc7       @TODO
   EXT16        = 0b11001000, // 0xc8       @TODO
   EXT32        = 0b11001001, // 0xc9       @TODO
   FLOAT32      = 0b11001010, // 0xca       @TODO
   FLOAT64      = 0b11001011, // 0xcb       @TODO
   UINT8        = 0b11001100, // 0xcc       @TODO
   UINT16       = 0b11001101, // 0xcd       @TODO
   UINT32       = 0b11001110, // 0xce       @TODO
   UINT64       = 0b11001111, // 0xcf       @TODO
   INT8         = 0b11010000, // 0xd0       @TODO
   INT16        = 0b11001101, // 0xd1       @TODO
   INT32        = 0b11001110, // 0xd2       @TODO
   INT64        = 0b11001111, // 0xd3       @TODO
   FIXEXT1      = 0b11010100, // 0xd4       @TODO
   FIXEXT2      = 0b11010101, // 0xd5       @TODO
   FIXEXT4      = 0b11010110, // 0xd6       @TODO
   FIXEXT8      = 0b11010111, // 0xd7       @TODO
   FIXEXT16     = 0b11011000, // 0xd8       @TODO
   STR8         = 0b11011001, // 0xd9       @TODO
   STR16        = 0b11011010, // 0xda       @TODO
   STR32        = 0b11011011, // 0xdb       @TODO
   ARR16        = 0b11011100, // 0xdc       @TODO
   ARR32        = 0b11011101, // 0xdd       @TODO
   MAP16        = 0b11011110, // 0xde       @TODO
   MAP32        = 0b11011111, // 0xdf       @TODO
};
// clang-format on

/*****************************************************************************************
 ************************************   Serializers   ************************************
 ****************************************************************************************/

/**
 * @brief Serializes a boolean value of true or false.
 * 
 * @param val The value to serialize
 * @return ByteArray An array containing the serialized byte(s).
 */
inline ByteArray Serialize(bool val) {
   return (val) ? ByteArray {Formats::BTRUE} : ByteArray {Formats::BFALSE};
}

/*****************************************************************************************
 ***********************************   Deserializers   ***********************************
 ****************************************************************************************/

/**
 * @brief Recursively deserializes any unrecognized, user-defined type.
 * 
 * @tparam T The user-defined type to deserialize.
 * @param data The data stream to deserialize.
 * @return T A deserialized instance of the T data type.
 */
template<typename T>
T Deserialize(ByteArray data) {
   /** @TODO */
}

/**
 * @brief Deserializes a boolean value.
 * 
 * @param data The data stream to deserialize.
 * @throws std::invalid_argument if the data stream is empty.
 * @throws std::runtime_error if the data stream's format does not match the 
 * template specialization
 * @return The deserialized boolean value (true/false).
 */
template<>
bool Deserialize<bool>(ByteArray data) {
   if (data.empty()) { throw std::invalid_argument("Empty ByteArray"); }

   switch (data[0]) {
      case Formats::BTRUE: {
         return true;
      }
      case Formats::BFALSE: {
         return false;
      }
      default: {
         throw std::runtime_error("ByteArray does not match type bool");
      }
   }
}

}; // namespace pack