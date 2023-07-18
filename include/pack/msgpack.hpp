#pragma once

#include <numeric>
#include <cstdint>
#include <vector>
#include <stdexcept>
#include <ostream>
#include <istream>
#include <bit>
#include <string>
#include <span>

namespace pack {

// Requires cpp20
// Requires big or little endian architecture
// Requires twos-complement architecture
// Requires IEEE 754 floating point precision types
static_assert(0xFF == (uint8_t)-1);
static_assert(std::numeric_limits<double>::is_iec559);
static_assert(sizeof(float) == 4);
static_assert(sizeof(double) == 8);

/*****************************************************************************************
 ***********************************   Msgpack Defs   ************************************
 ****************************************************************************************/
// clang-format off
using Byte                          = uint8_t;
using ByteArray                     = std::vector<Byte>;
constexpr uint8_t POS_FIXINT_MAX    = 0b1111111;
constexpr int8_t NEG_FIXINT_MIN     = 0b11100000;
constexpr uint8_t POS_FIXINT_MASK   = 0b10000000;
constexpr uint8_t FIXSTR_MASK       = 0b10100000;
constexpr uint8_t FIXARR_MASK       = 0b10010000;

constexpr uint8_t FIXSTR_MAX        = 0b11111;

enum Formats : Byte {
   POS_FIXINT   = 0b00000000, // 0XXXXXXX
   NEG_FIXINT   = 0b11100000, // 111xxxxx
   FIXMAP       = 0b10000000, // 1000xxxx   @TODO
   FIXARR       = 0b10010000, // 1001xxxx
   FIXSTR       = 0b10100000, // 101xxxxx

   NIL          = 0b11000000, // 0xc0       @TODO
   BFALSE       = 0b11000010, // 0xc2
   BTRUE        = 0b11000011, // 0xc3
   EXT8         = 0b11000111, // 0xc7       @TODO
   EXT16        = 0b11001000, // 0xc8       @TODO
   EXT32        = 0b11001001, // 0xc9       @TODO
   FLOAT32      = 0b11001010, // 0xca
   FLOAT64      = 0b11001011, // 0xcb
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
   STR8         = 0b11011001, // 0xd9
   STR16        = 0b11011010, // 0xda
   STR32        = 0b11011011, // 0xdb
   ARR16        = 0b11011100, // 0xdc
   ARR32        = 0b11011101, // 0xdd
   MAP16        = 0b11011110, // 0xde       @TODO
   MAP32        = 0b11011111, // 0xdf       @TODO
};

/*****************************************************************************************
 *************************************   Concepts   **************************************
 ****************************************************************************************/
template<class T, class U>
concept IsType = std::same_as<T, U>;

template<class T>
concept UnsignedInt = std::is_unsigned_v<T> && !(std::same_as<T, bool>);

template<class T>
concept SignedInt =
    std::is_signed_v<T> && !(std::same_as<T, bool>) && !(std::floating_point<T>);

template<class T>
concept StringType = std::convertible_to<T, std::string_view>;

template<class T>
concept ArrayType = requires(T &a) { { std::span(a) }; } && !StringType<T>;

// TODO: ResizableArray concept

// clang-format on

/*****************************************************************************************
 *********************************   Byte Utilities   ************************************
 ****************************************************************************************/
template<typename T>
requires std::has_unique_object_representations_v<T>
T Byteswap(T value) {
   auto temp = std::bit_cast<std::array<uint8_t, sizeof(T)>>(value);
   std::ranges::reverse(temp);
   return std::bit_cast<T>(temp);
}

template<typename T>
requires std::has_unique_object_representations_v<T>
T ToBigEndian(T in) {
   if constexpr (std::endian::native == std::endian::little) {
      return Byteswap(in);
   } else {
      return in;
   }
}

template<typename T>
requires std::has_unique_object_representations_v<T>
T ToLittleEndian(T in) {
   if constexpr (std::endian::native == std::endian::little) {
      return Byteswap(in);
   } else {
      return in;
   }
}

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
   void Serialize(const T &next, const Rest &...rest) {
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
      } else if (val >= 0 && val <= POS_FIXINT_MAX) {
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

   /**
    * @brief Serialize a single UTF-8 null-terminated string.
    * 
    * @tparam T The string type to serialize (fixed length char array, std::string)
    * @param val The data to serialize.
    * @throws std::runtime_error if there was a failure writing to the stream.
    */
   template<typename T>
   requires StringType<T>
   void Serialize(const T &val) {
      std::string_view view(val);
      if (view.length() > UINT32_MAX) {
         throw std::length_error("String exceeds max length");
      } else if (view.length() <= FIXSTR_MAX) {
         uint8_t fmt = FIXSTR_MASK | view.length();
         mRef.put(fmt);
         mRef.write(view.data(), view.length());
      } else if (view.length() <= UINT8_MAX) {
         mRef.put(Formats::STR8);
         mRef.put(view.length());
         mRef.write(view.data(), view.length());
      } else if (view.length() <= UINT16_MAX) {
         mRef.put(Formats::STR16);
         uint16_t lenBigEndian = ToBigEndian((uint16_t)view.length());
         mRef.write((char *)&lenBigEndian, 2);
         mRef.write(view.data(), view.length());
      } else {
         mRef.put(Formats::STR32);
         uint32_t lenBigEndian = ToBigEndian((uint32_t)view.length());
         mRef.write((char *)&lenBigEndian, 4);
         mRef.write(view.data(), view.length());
      }

      if (mRef.fail()) {
         mRef.clear();
         throw std::runtime_error("stream write error");
      };
   }

   /**
    * @brief Serialize a double precision IEEE 754 floating value.
    * 
    * @param val The data to serialize.
    * @throws std::runtime_error if there was a failure writing to the stream.
    */
   template<typename T>
   requires IsType<T, double>
   void Serialize(T val) {
      uint64_t data = 0;
      memcpy(&data, &val, 8);
      data = ToBigEndian(data);
      mRef.put(Formats::FLOAT64);
      mRef.write((char *)&data, 8);

      if (mRef.fail()) {
         mRef.clear();
         throw std::runtime_error("stream write error");
      };
   }

   /**
    * @brief Serialize a single precision IEEE 754 floating value.
    * 
    * @param val The data to serialize.
    * @throws std::runtime_error if there was a failure writing to the stream.
    */
   template<typename T>
   requires IsType<T, float>
   void Serialize(T val) {
      uint32_t data = 0;
      memcpy(&data, &val, 4);
      data = ToBigEndian(data);
      mRef.put(Formats::FLOAT32);
      mRef.write((char *)&data, 4);

      if (mRef.fail()) {
         mRef.clear();
         throw std::runtime_error("stream write error");
      };
   }

   template<typename T, size_t N>
   requires(not IsType<T, char const>)
   void Serialize(T (&arr)[N]) {
      Serialize(std::span<T, N>(arr));
   }

   template<typename T>
   requires ArrayType<T>
   void Serialize(T arr) {
      auto span = std::span(arr);
      auto sz = span.size();

      if (span.size() <= 15) {
         uint8_t fmt = FIXARR_MASK | span.size();
         mRef.put(fmt);

         for (auto element : span) { Serialize(element); }
      } else if (span.size() <= UINT16_MAX) {
         mRef.put(Formats::ARR16);
         uint64_t bigEndian = ToBigEndian((uint16_t)span.size());
         mRef.write((char *)&bigEndian, 2);

         for (auto element : span) { Serialize(element); }
      } else if (span.size() <= UINT32_MAX) {
         mRef.put(Formats::ARR32);
         uint64_t bigEndian = ToBigEndian((uint32_t)span.size());
         mRef.write((char *)&bigEndian, 4);

         for (auto element : span) { Serialize(element); }
      } else {
         throw std::invalid_argument("Array exceeds max allowable size");
      }
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
            } else if ((fmtOrData & POS_FIXINT_MASK) == 0) {
               mRef.get(fmtOrData);
               out = (int8_t)fmtOrData;
               break;
            } else {
               throw std::runtime_error("ByteArray does not match type int");
            }
         }
      }
   }

   /**
    * @brief Deserializes a UTF-8 string into a fixed-size C-style character array.
    * 
    * The fixed length array must contain enough space for each deserialized UTF-8
    * character as well as a null-terminator which is automatically appended by this 
    * function.
    * 
    * @tparam N The number of bytes that the fixed length character array can hold.
    * @throws std::invalid_argument If there are no more bytes in the stream.
    * @throws std::length_error if the array is too small to hold each deserialized 
    * byte plus a null terminator.
    * @throws std::runtime_error if the bytestream data does not encode a string.
    */
   template<size_t N>
   void Deserialize(char (&str)[N]) {
      if (mRef.peek() == EOF) {
         mRef.clear();
         throw std::invalid_argument("No more data to read");
      }

      char fmt = Formats::NIL;
      mRef.get(fmt);
      switch ((Formats)fmt) {
         case STR8: {
            uint8_t len = 0;
            mRef.get((char &)len);
            if (N < len + 1) {
               mRef.unget();
               throw std::length_error("Char array too small");
            }
            mRef.read(str, len);
            str[len] = '\0';
            break;
         }
         case STR16: {
            uint16_t len = 0;
            mRef.read((char *)&len, 2);
            len = ToLittleEndian(len);
            if (N < len + 1) {
               mRef.unget();
               throw std::length_error("Char array too small");
            }
            mRef.read(str, len);
            str[len] = '\0';
            break;
         }
         case STR32: {
            uint32_t len = 0;
            mRef.read((char *)&len, 4);
            len = ToLittleEndian(len);
            if (N < len + 1) {
               mRef.unget();
               throw std::length_error("Char array too small");
            }
            mRef.read(str, len);
            str[len] = '\0';
            break;
         }
         default: {
            if ((fmt & FIXSTR_MASK) == FIXSTR_MASK) {
               uint8_t len = fmt & FIXSTR_MAX;
               if (N < len + 1) {
                  mRef.unget();
                  throw std::length_error("Char array too small");
               }
               mRef.read(str, len);
               str[len] = '\0';
               break;
            } else {
               throw std::runtime_error("ByteArray does not match type String");
            }
         }
      }
   }

   /**
    * @brief Deserializes a UTF-8 string.
    * 
    * @throws std::invalid_argument If there are no more bytes in the stream.
    * @throws std::runtime_error if the bytestream data does not encode a string.
    */
   template<typename T>
   requires IsType<T, std::string>
   void Deserialize(T &out) {
      if (mRef.peek() == EOF) {
         mRef.clear();
         throw std::invalid_argument("No more data to read");
      }

      char fmt = Formats::NIL;
      fmt = mRef.get();
      switch ((Formats)fmt) {
         case STR8: {
            mRef.get(fmt);
            uint8_t len = (uint8_t)fmt;
            out.resize((uint8_t)len);
            mRef.read(out.data(), len);
            out.append(1, '\0');
            break;
         }
         case STR16: {
            uint16_t len = 0;
            mRef.read((char *)&len, 2);
            len = ToLittleEndian(len);
            out.resize(len);
            mRef.read(out.data(), len);
            out.append(1, '\0');
            break;
         }
         case STR32: {
            uint32_t len = 0;
            mRef.read((char *)&len, 4);
            len = ToLittleEndian(len);
            out.resize(len);
            mRef.read(out.data(), len);
            out.append(1, '\0');
            break;
         }
         default: {
            if ((fmt & FIXSTR_MASK) == FIXSTR_MASK) {
               uint8_t len = fmt & FIXSTR_MAX;
               out.resize(len);
               mRef.read(out.data(), len);
               out.append(1, '\0');
               break;
            } else {
               throw std::runtime_error("ByteArray does not match type String");
            }
         }
      }
   }

   /**
    * @brief Deserializes a IEEE 754 floating point value.
    * 
    * @tparam T The type (float, double) of the out parameter to deserialize into.
    * @param out The location to place the deserialized data.
    * @throws std::invalid_argument If there are no more bytes in the stream.
    * @throws std::runtime_error if the bytestream data does not encode a string.
    * @throws std::length_error If deserializing the data into T would result in 
    * loss of precision.
    */
   template<typename T>
   requires std::floating_point<T>
   void Deserialize(T &out) {
      if (mRef.peek() == EOF) {
         mRef.clear();
         throw std::invalid_argument("No more data to read");
      }
      out = 0;

      char fmt = Formats::NIL;
      fmt = mRef.get();
      switch ((Formats)fmt) {
         case Formats::FLOAT32: {
            if (std::numeric_limits<T>::max() < std::numeric_limits<float>::max()) {
               throw std::length_error("Narrowing conversion");
            }
            uint32_t data = 0;
            mRef.read((char *)&data, 4);
            data = ToLittleEndian(data);
            memcpy(&out, &data, 4);
            break;
         }
         case Formats::FLOAT64: {
            if (std::numeric_limits<T>::max() < std::numeric_limits<double>::max()) {
               throw std::length_error("Narrowing conversion");
            }
            uint64_t data = 0;
            mRef.read((char *)&data, 8);
            data = ToLittleEndian(data);
            memcpy(&out, &data, 8);
            break;
         }
         default: {
            throw std::runtime_error("ByteArray does not match type float");
         }
      }
   }

   template<typename T, size_t N>
   requires(not IsType<T, char const>)
   void Deserialize(T (&arr)[N]) {
      Deserialize(arr, N);
   }

   template<typename T>
   requires ArrayType<T>
   void Deserialize(T &out) {
      size_t len = std::span(out).size();
      Deserialize(out, len);
   }

   template<typename T>
   requires ArrayType<T>
   void Deserialize(T &out, size_t outputLen) {
      if (mRef.peek() == EOF) {
         mRef.clear();
         throw std::invalid_argument("No more data to read");
      }

      char fmt = Formats::NIL;
      fmt = mRef.peek(); // Nondestructive peek

      switch ((Formats)fmt) {
         case Formats::ARR16: {
            mRef.get(fmt); // pop the specifier
            uint16_t arrLen = ToLittleEndian(PeekMultiBytesUint<uint16_t>());

            if (arrLen > outputLen) {
               mRef.unget(); // Put the format specifier back
               throw std::length_error("Input array is not large enough");
            }

            // Can safely modify more than 1 byte of the stream now.
            mRef.ignore(2);

            for (uint16_t i = 0; i < arrLen; i++) { Deserialize(out[i]); }
            break;
         }
         case Formats::ARR32: {
            mRef.get(fmt); // pop the specifier
            uint32_t arrLen = ToLittleEndian(PeekMultiBytesUint<uint32_t>());

            if (arrLen > outputLen) {
               mRef.unget(); // Put the format specifier back
               throw std::length_error("Input array is not large enough");
            }

            // Can safely modify more than 1 byte of the stream now.
            mRef.ignore(4);

            for (uint32_t i = 0; i < arrLen; i++) { Deserialize(out[i]); }
            break;
         }
         default: {
            if ((fmt & FIXARR_MASK) == FIXARR_MASK) {
               uint8_t arrLen = fmt & 0b1111;

               if (arrLen > outputLen) {
                  throw std::length_error("Input array is not large enough");
               }

               mRef.get(fmt); // pop the specifier
               for (uint8_t i = 0; i < arrLen; i++) { Deserialize(out[i]); }
            } else {
               throw std::runtime_error("ByteArray does not match type array");
            }
         }
      }
   }

   template<typename T>
   void Deserialize(std::vector<T> &out) {
      if (mRef.peek() == EOF) {
         mRef.clear();
         throw std::invalid_argument("No more data to read");
      }

      char fmt = Formats::NIL;
      fmt = mRef.peek(); // Nondestructive peek

      switch ((Formats)fmt) {
         case Formats::ARR16: {
            mRef.get(fmt); // pop the specifier
            uint16_t arrLen = ToLittleEndian(PeekMultiBytesUint<uint16_t>());

            // Can safely modify more than 1 byte of the stream now.
            mRef.ignore(2);

            for (uint16_t i = 0; i < arrLen; i++) {
               out.resize(arrLen);
               Deserialize(out[i]);
            }
            break;
         }
         case Formats::ARR32: {
            mRef.get(fmt); // pop the specifier
            uint32_t arrLen = ToLittleEndian(PeekMultiBytesUint<uint32_t>());

            // Can safely modify more than 1 byte of the stream now.
            mRef.ignore(4);

            for (uint32_t i = 0; i < arrLen; i++) {
               out.resize(arrLen);
               Deserialize(out[i]);
            }
            break;
         }
         default: {
            if ((fmt & FIXARR_MASK) == FIXARR_MASK) {
               uint8_t arrLen = fmt & 0b1111;

               mRef.get(fmt); // pop the specifier
               for (uint8_t i = 0; i < arrLen; i++) {
                  out.resize(arrLen);
                  Deserialize(out[i]);
               }
            } else {
               throw std::runtime_error("ByteArray does not match type array");
            }
         }
      }
   }

  private:
   template<typename T>
   T PeekMultiBytesUint() {
      std::array<Byte, sizeof(T)> arr;
      std::streampos save = mRef.tellg();

      for (size_t i = 0; i < sizeof(T); i++) {
         mRef.seekg(save + std::streamoff(i));
         arr[i] = mRef.peek();
      }

      mRef.seekg(save);
      return std::bit_cast<T>(arr);
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

   size_t mStreamStart {0};
   std::istream &mRef;
};
}; // namespace pack