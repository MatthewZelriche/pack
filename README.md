<h1 align="center">Pack</h1>

<div align="center">
A header-only implementation of the MessagePack serialization specification, written in modern C++
</div>

## Implementation Progress

| Data Type          | Implemented? |
|--------------------|---------------------|
| Booleans           | :white_check_mark:  |
| Signed Integers    | :white_check_mark:  |
| Unsigned Integers  | :white_check_mark:  |
| Floats             | :white_check_mark:  |
| Arrays             | :white_check_mark:  |
| Strings            | :white_check_mark:  |
| Maps               | :x:                 |
| Extension          | :x:                 |
| Nil                | :x:                 |

### Standard Library Containers

Pack provides limited support for serializing (but not necessarily deserializing) standard library containers through concepts. 

* Anything convertible to `std::span` can be serialized as an Array (`std::vector`, etc)
* Anything convertible to `std::string_view` can be serialized as a String (`std::string`, null-terminated `const char *`, etc)

## Getting Started

As a header-only library, using Pack is as simple as including the header file in your project. CMake is necessary only for building the unit tests. 

The public interface for the library is designed to be familiar for anyone who has utilized the excellent [Cereal](https://uscilab.github.io/cereal/) library. Usage revolves around the `Packer` and `Unpacker` classes that are constructed with some kind of c++ stream. Note that similarly to Cereal, the stream is not 
flushed until the destructor is called: 

```
   std::stringstream stream(std::ios::binary | std::ios::out | std::ios::in);

   {
      pack::Packer packer(stream);
      packer.Serialize(data1, data2, ...);
   }

   // Later...
   {
      pack::Unpacker unpacker(stream);
      unpacker.Deserialize(data1, data2, ...);
   }
```

## Licensing Information

This project is licensed under the MIT License. See the LICENSE file for details. 


