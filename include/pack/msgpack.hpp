#pragma once

#include <cstdint>
#include <vector>
#include <stdexcept>
#include <ostream>
#include <istream>
#include <bit>

// Requires cpp20
// Requires big or little endian architecture
// Requires twos-complement architecture
static_assert(0xFF == (uint8_t)-1);
static_assert(std::endian::native == std::endian::little ||
              std::endian::native == std::endian::big);

/*****************************************************************************************
 ********************************   Endian Converters   **********************************
 ****************************************************************************************/
#define PACK_BIG_ENDIAN_FN(type, fnName)                          \
   inline type ToBigEndian(type in) {                             \
      if constexpr (std::endian::native == std::endian::little) { \
         return fnName(in);                                       \
      } else {                                                    \
         return in;                                               \
      }                                                           \
   }

#define PACK_LITTLE_ENDIAN_FN(type, fnName)                    \
   inline type ToLittleEndian(type in) {                       \
      if constexpr (std::endian::native == std::endian::big) { \
         return fnName(in);                                    \
      } else {                                                 \
         return in;                                            \
      }                                                        \
   }

namespace pack {

#ifdef _MSC_VER
#include <stdlib.h>
PACK_BIG_ENDIAN_FN(uint16_t, _byteswap_ushort)
PACK_BIG_ENDIAN_FN(uint32_t, _byteswap_ulong)
PACK_BIG_ENDIAN_FN(uint64_t, _byteswap_uint64)
PACK_LITTLE_ENDIAN_FN(uint16_t, _byteswap_ushort)
PACK_LITTLE_ENDIAN_FN(uint32_t, _byteswap_ulong)
PACK_LITTLE_ENDIAN_FN(uint64_t, _byteswap_uint64)
#else
#include <byteswap.h>
PACK_BIG_ENDIAN_FN(uint16_t, bswap_16)
PACK_BIG_ENDIAN_FN(uint32_t, bswap_32)
PACK_BIG_ENDIAN_FN(uint64_t, bswap_64)
PACK_LITTLE_ENDIAN_FN(uint16_t, bswap_16)
PACK_LITTLE_ENDIAN_FN(uint32_t, bswap_32)
PACK_LITTLE_ENDIAN_FN(uint64_t, bswap_64)
#endif

/*****************************************************************************************
 ***********************************   Msgpack Defs   ************************************
 ****************************************************************************************/
// clang-format off
using Byte                          = uint8_t;
using ByteArray                     = std::vector<Byte>;
constexpr uint8_t POS_FIXINT_MAX    = 0b1111111;
constexpr uint8_t POS_FIXINT_MASK   = 0b10000000;

enum Formats : Byte {
   POS_FIXINT   = 0b00000000, // 0XXXXXXX
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
   UINT8        = 0b11001100, // 0xcc
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
 *************************************   Concepts   **************************************
 ****************************************************************************************/
template<class T, class U>
concept IsType = std::same_as<T, U>;

template<class T>
concept UnsignedInt = std::is_integral_v<T> && !(std::same_as<T, bool>);

/*****************************************************************************************
 **************************************   Classes   **************************************
 ****************************************************************************************/
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
      mStreamStart = mRef.tellp();
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
      mStreamStart = mRef.tellp();
   }

   ~Packer() { mRef.flush(); }

   /**
    * @brief Gets a count of the number of bytes that have been successfully serialized 
    * so far.
    * 
    * Note that just because bytes have been serialized, does not mean they have 
    * successfully been written. Serialized data is only guarunteed to be written out 
    * on a call to the destructor.
    * 
    * @return size_t The number of bytes successfully serialized so far.
    */
   size_t ByteCount() { return (uint64_t)mRef.tellp() - mStreamStart; }

   /**
    * @brief Serialize a single boolean value to the bytestream.
    * 
    * @param val The value to serialize
    * @returns true if serializing to the bytestream was successful, false otherwise.
    */
   template<typename T>
   requires IsType<T, bool> bool
   Serialize(T val) {
      char data = val ? Formats::BTRUE : Formats::BFALSE;
      mRef.put(data);
      if (mRef.fail()) {
         mRef.clear();
         return false;
      }
      return true;
   }

   /**
    * @brief Serialize a single unsigned integer to the bytestream
    * 
    * Can serialize 8, 16, 32, 64 bit unsigned integers into 1, 2, 3, 5, or 9 bytes of 
    * serialized data.
    * 
    * @tparam T The unsigned integer width type to serialize
    * @param val The value to serialize
    * @return true if serializing to the bytestream was successful, false otherwise.
    */
   template<typename T>
   requires UnsignedInt<T> bool
   Serialize(T val) {
      if (val <= POS_FIXINT_MAX) {
         mRef.put(val);
      } else if (val <= UINT8_MAX) {
         mRef.put(Formats::UINT8);
         mRef.put(val);
      } else if (val <= UINT16_MAX) {
         /** @TODO */
      } else if (val <= UINT32_MAX) {
         /** @TODO */
      } else {
         /** @TODO */
      }

      if (mRef.fail()) {
         mRef.clear();
         return false;
      };
      return true;
   }

  private:
   size_t mStreamStart {0};
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
      mStreamStart = mRef.tellg();
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
      mStreamStart = mRef.tellg();
   }

   /**
    * @brief Gets a count of the number of bytes of serialized data that have been 
    * successfully read in so far.
    * 
    * Note that the number of bytes deserialized is not the same as the number of bytes 
    * actually returned by Deserialize.
    * 
    * @return size_t The number of bytes successfully deserialized so far.
    */
   size_t ByteCount() { return (uint64_t)mRef.tellg() - mStreamStart; }

   /**
    * @brief Deserializes a single boolean value.
    * 
    * @throws std::invalid_argument if there is no additional bytestream data.
    * @throws std::runtime_error if the bytestream data does not encode a boolean.
    * @returns The deserialized boolean value.
    */
   template<typename T>
   requires IsType<T, bool> bool
   Deserialize() {
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

   /**
    * @brief Deserializes a single 8 bit unsigned integer value.
    * 
    * @return The deserialized value.
    */
   template<typename T>
   requires IsType<T, uint8_t>
   T Deserialize() {
      if (mRef.rdbuf()->in_avail() == 0) {
         throw std::invalid_argument("No more data to read");
      }

      char fmtOrData = Formats::NIL;
      mRef.get(fmtOrData);
      switch ((Formats)fmtOrData) {
         case Formats::UINT8: {
            mRef.get(fmtOrData);
            return (T)fmtOrData;
         }
         default: {
            if ((fmtOrData & POS_FIXINT_MASK) == 0) {
               // Positive fixint
               return (T)fmtOrData;
            } else {
               throw std::runtime_error("ByteArray does not match type uint");
            }
         }
      }
   }

  private:
   size_t mStreamStart {0};
   std::istream &mRef;
};

}; // namespace pack