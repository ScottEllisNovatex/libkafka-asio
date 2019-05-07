//
// primitives.h
// ------------
//
// Copyright (c) 2015 Daniel Joos
//
// Distributed under MIT license. (See file LICENSE)
//

#ifndef PRIMITIVES_H_2018391E_E5C4_4FB1_8271_CAD5C2C99951
#define PRIMITIVES_H_2018391E_E5C4_4FB1_8271_CAD5C2C99951

#include <string>
#include <vector>

namespace libkafka_asio
{

//
// The Kafka protocol primitives, as described in the Kafka wiki.
//

typedef int8_t Int8;
typedef int16_t Int16;
typedef int32_t Int32;
typedef int64_t Int64;
typedef uint8_t Byte;
typedef std::string String;
typedef std::shared_ptr<std::vector<Byte> > Bytes;

}  // namespace libkafka_asio

#endif  // PRIMITIVES_H_2018391E_E5C4_4FB1_8271_CAD5C2C99951
