#pragma once

#include <cstdint>
#include <vector>
#include <stdexcept>
#include <ostream>
#include <istream>

namespace pack {

// Just in case, only two's complement is supported...
static_assert(0xFF == (uint8_t)-1);

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

class Packer {
  public:
   /**
   * @brief Construct a new Packer object, setting stream to the beginning of the 
   * buffer.
   * 
   * @param stream The byte stream to pack serialized data out to. Must have the 
   * std::ios::binary and std::ios::out mode flags set.
   */
   Packer(std::ostream &stream) : mRef(stream) {
      mRef.seekp(std::ios::beg);
      mStreamPos = mRef.tellp();
   }

   /**
   * @brief Construct a new Packer object, setting the stream to a specified start 
   * position.
   * 
   * Useful if you want to serialize data out to a specified position in a file.
   * 
   * @param stream The byte stream to pack serialized data out to. Must have the 
   * std::ios::binary and std::ios::out mode flags set.
   * @param start The start offset, in bytes, from the beginning of the stream.
   */
   Packer(std::ostream &stream, size_t start) : mRef(stream) {
      mRef.seekp(start);
      mStreamPos = mRef.tellp();
   }

   ~Packer() { mRef.flush(); }

   /**
    * @brief Serialize some value to the byte stream.
    * 
    * @tparam T The type to serialize
    * @param val The data to serialize.
    * @returns true if the data was successfully serialized, false if an error occured. 
    * Errors can occur either during the serialization process, or while writing to the 
    * stream.
    */
   template<typename T>
   bool Serialize(T val) {
      return SerializeImpl(val);
   }

  private:
   /**
    * @brief Serialize a single boolean value to the bytestream.
    * 
    * @param val The value to serialize
    * @returns true if serializing to the bytestream was successfull, false otherwise.
    */
   bool SerializeImpl(bool val) {
      char data = val ? Formats::BTRUE : Formats::BFALSE;
      mRef.put(data);
      if (mRef.fail()) { return false; }
      mStreamPos = mRef.tellp();
      return true;
   }

   size_t mStreamPos {0};
   std::ostream &mRef;
};

class Unpacker {
  public:
   /**
   * @brief Construct a new Unpacker object, setting stream to the beginning of the
   * buffer.
   * 
   * @param stream The byte stream to unpack serialized data from. Must have the 
   * std::ios::binary and std::ios::in mode flags set.
   */
   Unpacker(std::istream &stream) : mRef(stream) {
      mRef.seekg(mRef.beg);
      mStreamPos = mRef.tellg();
   }

   /**
   * @brief Construct a new Unpacker object, setting the stream to a specified start 
   * position.
   * 
   * @param stream The byte stream to unpack serialized data from. Must have the 
   * std::ios::binary and std::ios::in mode flags set.
   */
   Unpacker(std::istream &stream, size_t start) : mRef(stream) {
      mRef.seekg(start);
      mStreamPos = mRef.tellg();
   }

   /**
    * @brief Deserializes a value from the byte stream.
    * 
    * @tparam T The type to deserialize.
    * @throws std::invalid_argument if there is no additional bytestream data.
    * @throws std::runtime_error if the bytestream format does not match the requested 
    * type T.
    * @return T The deserialized type instance.
    */
   template<typename T>
   T Deserialize() {
      return DeserializeImpl<T>();
   }

  private:
   /**
    * @brief Deserializes a user-defined type.
    * 
    * @tparam T The type to deserialize. 
    * @return T The deserialized type instance.
    */
   template<typename T>
   T DeserializeImpl() {
      /** @TODO */
   }

   /**
    * @brief Deserializes a single boolean value.
    * 
    * @throws std::invalid_argument if there is no additional bytestream data.
    * @throws std::runtime_error if the bytestream data does not encode a boolean.
    * @returns The deserialized boolean value.
    */
   template<>
   bool DeserializeImpl<bool>() {
      if (mRef.rdbuf()->in_avail() == 0) {
         throw std::invalid_argument("No more data to read");
      }

      char data = Formats::NIL;
      mRef.get(data);

      switch ((Formats)data) {
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

   size_t mStreamPos {0};
   std::istream &mRef;
};

}; // namespace pack