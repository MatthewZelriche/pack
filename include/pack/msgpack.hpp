#pragma once

#include <numeric>
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

#define PACK_LITTLE_ENDIAN_FN(type, fnName)                       \
   inline type ToLittleEndian(type in) {                          \
      if constexpr (std::endian::native == std::endian::little) { \
         return fnName(in);                                       \
      } else {                                                    \
         return in;                                               \
      }                                                           \
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
constexpr int8_t NEG_FIXINT_MIN     = 0b11100000;
constexpr uint8_t POS_FIXINT_MASK   = 0b10000000;

enum Formats : Byte {
   POS_FIXINT   = 0b00000000, // 0XXXXXXX
   NEG_FIXINT   = 0b11100000, // 111xxxxx
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
   UINT16       = 0b11001101, // 0xcd
   UINT32       = 0b11001110, // 0xce
   UINT64       = 0b11001111, // 0xcf
   INT8         = 0b11010000, // 0xd0
   INT16        = 0b11010001, // 0xd1
   INT32        = 0b11010010, // 0xd2
   INT64        = 0b11010011, // 0xd3
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
concept UnsignedInt = std::is_unsigned_v<T> && !(std::same_as<T, bool>);

template<class T>
concept SignedInt = std::is_signed_v<T> && !(std::same_as<T, bool>);

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
    * @brief Serializes any number of values to the bytestream.
    * 
    * @tparam T The type of the first value to serialize.
    * @tparam Rest The rest of the types to serialize.
    * @param next The next value to serialize.
    * @param rest A parameter pack consisting of the rest of the values.
    */
   template<typename T, typename... Rest>
   void Serialize(T next, Rest... rest) {
      Serialize(next);
      Serialize(rest...);
   }

   /**
    * @brief Serialize a single boolean value to the bytestream.
    * 
    * @param val The value to serialize
    * @throws std::runtime_error if there was a failure writing to the stream.
    */
   template<typename T>
   requires IsType<T, bool>
   void Serialize(T val) {
      char data = val ? Formats::BTRUE : Formats::BFALSE;
      mRef.put(data);
      if (mRef.fail()) {
         mRef.clear();
         throw std::runtime_error("stream write error");
      }
   }

   /**
    * @brief Serialize a single unsigned integer to the bytestream
    * 
    * Can serialize 8, 16, 32, 64 bit unsigned integers into 1, 2, 3, 5, or 9 bytes of 
    * serialized data.
    * 
    * @tparam T The unsigned integer width type to serialize
    * @param val The value to serialize
    * @throws std::runtime_error if there was a failure writing to the stream.
    */
   template<typename T>
   requires UnsignedInt<T>
   void Serialize(T val) {
      if (val <= POS_FIXINT_MAX) {
         mRef.put(val);
      } else if (val <= UINT8_MAX) {
         mRef.put(Formats::UINT8);
         mRef.put(val);
      } else if (val <= UINT16_MAX) {
         mRef.put(Formats::UINT16);
         uint16_t convert = ToBigEndian((uint16_t)val);
         mRef.write((char *)&convert, 2);
      } else if (val <= UINT32_MAX) {
         mRef.put(Formats::UINT32);
         uint32_t convert = ToBigEndian((uint32_t)val);
         mRef.write((char *)&convert, 4);
      } else {
         mRef.put(Formats::UINT64);
         uint64_t convert = ToBigEndian((uint64_t)val);
         mRef.write((char *)&convert, 8);
      }

      if (mRef.fail()) {
         mRef.clear();
         throw std::runtime_error("stream write error");
      };
   }

   /**
    * @brief Serialize a single signed integer to the bytestream.
    * 
    * Can serialize 8, 16, 32, 64 bit signed integers into 1, 2, 3, 5, or 9 bytes of 
    * serialized data.
    * 
    * @tparam T The unsigned integer width type to serialize
    * @param val The value to serialize
    * @throws std::runtime_error if there was a failure writing to the stream.
    */
   template<typename T>
   requires SignedInt<T>
   void Serialize(T val) {
      if (val < 0 && val >= NEG_FIXINT_MIN) {
         mRef.put(val);
      } else if (val <= INT8_MAX && val >= INT8_MIN) {
         mRef.put(Formats::INT8);
         mRef.put(val);
      } else if (val <= INT16_MAX && val >= INT16_MIN) {
         mRef.put(Formats::INT16);
         int16_t convert = ToBigEndian((uint16_t)val);
         mRef.write((char *)&convert, 2);
      } else if (val <= INT32_MAX && val >= INT32_MIN) {
         mRef.put(Formats::INT32);
         int32_t convert = ToBigEndian((uint32_t)val);
         mRef.write((char *)&convert, 4);
      } else {
         mRef.put(Formats::INT64);
         int64_t convert = ToBigEndian((uint64_t)val);
         mRef.write((char *)&convert, 8);
      }

      if (mRef.fail()) {
         mRef.clear();
         throw std::runtime_error("stream write error");
      };
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
   * Useful if you want to deserialize data not at the beginning of a file.
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
    * @brief Deserializes a variable number of values.
    * 
    * @throws std::invalid_argument if there are no more bytes to read in the stream.
    * @throws std::runtime_error If a given type does not match its corresponding format 
    * specifier.
    * @throws std::length_error If deserializing the data into T would result in a 
    * narrowing conversion. (eg, Deserialized data is UINT64 but T is uint32_t)
    * @tparam T The first type to deserialize
    * @tparam Rest A parameter pack containing the rest of the types to deserialize.
    * @param next The next value to be filled with the deserialized data.
    * @param rest The rest of the values to be filled with deserialized data.
    */
   template<typename T, typename... Rest>
   void Deserialize(T &next, Rest &...rest) {
      Deserialize(next);
      Deserialize(rest...);
   }

   /**
    * @brief Deserializes a single boolean value.
    * 
    * @param out The value to be filled with the deserialized data.
    * @throws std::invalid_argument if the bytestream contains no more data.
    * @throws std::runtime_error if the bytestream data does not encode a boolean.
    * @returns The deserialized boolean value.
    */
   template<typename T>
   requires IsType<T, bool>
   void Deserialize(T &out) {
      if (mRef.peek() == EOF) {
         mRef.clear();
         throw std::invalid_argument("No more data to read");
      }

      char data = Formats::NIL;
      mRef.get(data);

      switch ((Formats)data) {
         case Formats::BTRUE: {
            out = true;
            break;
         }
         case Formats::BFALSE: {
            out = false;
            break;
         }
         default: {
            throw std::runtime_error("ByteArray does not match type bool");
         }
      }
   }

   /**
    * @brief Deserializes a single unsigned integer value of width 8, 16, 32, 64 bits.
    * 
    * @tparam T The type of the out parameter. T must be able to accomodate the 
    * deserialized value without narrowing conversions, else std::length_error is 
    * generated.
    * @param out The value to be filled with the deserialized data.
    * @throws std::invalid_argument if the bytestream contains no more data.
    * @throws std::runtime_error if the bytestream data does not encode an unsigned int.
    * @throws std::length_error If deserializing the data into T would result in a 
    * narrowing conversion.
    */
   template<typename T>
   requires UnsignedInt<T>
   void Deserialize(T &out) {
      if (mRef.peek() == EOF) {
         mRef.clear();
         throw std::invalid_argument("No more data to read");
      }
      // clear out param because it may have a larger width with extra data.
      out = 0;

      char fmtOrData = Formats::NIL;
      fmtOrData = mRef.peek(); // Nondestructive peek so we can forward

      switch ((Formats)fmtOrData) {
         case UINT8: {
            mRef.get(fmtOrData); // Pop the format specifier
            char data = 0;
            mRef.get(data);
            out = (uint8_t)data;
            break;
         }
         case UINT16: {
            ReadMultiByteUint<uint16_t>(out);
            break;
         }
         case UINT32: {
            ReadMultiByteUint<uint32_t>(out);
            break;
         }
         case UINT64: {
            ReadMultiByteUint<uint64_t>(out);
            break;
         }
         default: {
            if ((fmtOrData & POS_FIXINT_MASK) == 0) {
               // Positive fixint
               mRef.get(fmtOrData); // Pop out the stored val
               out = (uint8_t)fmtOrData;
               break;
            } else {
               throw std::runtime_error("ByteArray does not match type uint");
            }
         }
      }
   }

   /**
    * @brief Deserializes a single signed integer value of width 8, 16, 32, 64 bits.
    * 
    * @tparam T The type of the out parameter. T must be able to accomodate the 
    * deserialized value without narrowing conversions, else std::length_error is 
    * generated.
    * @param out The value to be filled with the deserialized data.
    * @throws std::invalid_argument if the bytestream contains no more data.
    * @throws std::runtime_error if the bytestream data does not encode a signed int.
    * @throws std::length_error If deserializing the data into T would result in a 
    * narrowing conversion.
    */
   template<typename T>
   requires SignedInt<T>
   void Deserialize(T &out) {
      if (mRef.peek() == EOF) {
         mRef.clear();
         throw std::invalid_argument("No more data to read");
      }
      // clear out param because it may have a larger width with extra data.
      out = 0;

      char fmtOrData = Formats::NIL;
      fmtOrData = mRef.peek(); // Nondestructive peek so we can forward
      switch ((Formats)fmtOrData) {
         case INT8: {
            mRef.get(fmtOrData); // Pop the format specifier
            char data = 0;
            mRef.get(data);
            out = (int8_t)data;
            break;
         }
         case INT16: {
            ReadMultiByteInt<int16_t>(out);
            break;
         }
         case INT32: {
            ReadMultiByteInt<int32_t>(out);
            break;
         }
         case INT64: {
            ReadMultiByteInt<int64_t>(out);
            break;
         }
         default: {
            if ((fmtOrData & NEG_FIXINT_MIN) == NEG_FIXINT_MIN) {
               // Negative fixint
               mRef.get(fmtOrData); // Pop out the stored val
               out = (int8_t)fmtOrData;
               break;
            } else {
               throw std::runtime_error("ByteArray does not match type int");
            }
         }
      }
   }

   /**
    * @brief Reads in a multibyte unsigned integer from the byte stream.
    * 
    * Type U must have a width capable of holding any possible value of type T.
    * 
    * @tparam T The C++ unsigned integral type matching the msgpack format specifier
    * @tparam U The unsigned integral type of the provided output parameter.
    * @param out The value to be filled with the read data.
    * @throws std::length_error if the width of type U is too small to accomodate any
    * value of type T. (ie, a narrowing conversion would occur)
    */
   template<typename T, typename U>
   void ReadMultiByteUint(U &out) {
      if (std::numeric_limits<U>::max() < std::numeric_limits<T>::max()) {
         throw std::length_error("Narrowing conversion");
      }

      char fmt;
      mRef.get(fmt); // Pop the format specifier
      T val = 0;
      mRef.read((char *)&val, sizeof(T));
      out = ToLittleEndian(val);
   }

   /**
    * @brief Reads in a multibyte signed integer from the byte stream.
    * 
    * @tparam T The C++ signed integral type matching the msgpack format specifier
    * @tparam U The signed integral type of the provided output parameter.
    * @param out The value to be filled with the read data.
    * @throws std::length_error if the width of type U is too small to accomodate any
    * value of type T. (ie, a narrowing conversion would occur)
    */
   template<typename T, typename U>
   void ReadMultiByteInt(U &out) {
      if (std::numeric_limits<U>::max() < std::numeric_limits<T>::max() ||
          (std::numeric_limits<U>::min() > std::numeric_limits<T>::min())) {
         throw std::length_error("Narrowing conversion");
      }

      char fmt;
      mRef.get(fmt); // Pop the format specifier
      T val = 0;
      size_t bbbb = sizeof(T);
      mRef.read((char *)&val, sizeof(T));
      out = (T)ToLittleEndian((std::make_unsigned_t<T>)val);
   }

  private:
   size_t mStreamStart {0};
   std::istream &mRef;
};

}; // namespace pack