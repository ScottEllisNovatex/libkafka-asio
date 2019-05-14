//
// detail/request_write_test.cpp
// -----------------------------
//
// Copyright (c) 2015 Daniel Joos
//
// Distributed under MIT license. (See file LICENSE)
//

#include <catch.hpp>
#include <libkafka_asio/libkafka_asio.h>
#include "StreamTest.h"

using namespace libkafka_asio;
using namespace libkafka_asio::detail;


TEST_CASE("RequestWriteTest.StringWireSize")
{
  String test_string1 = "Hello World";
  REQUIRE(2 + 11 == StringWireSize(test_string1));
  String test_string2 = "";
  REQUIRE(2 + 0 == StringWireSize(test_string2));
}

TEST_CASE("RequestWriteTest.BytesWireSize")
{
  Bytes test_bytes1;
  REQUIRE(4 + 0 == BytesWireSize(test_bytes1));
  Bytes test_bytes2(new Bytes::element_type());
  REQUIRE(4 + 0 == BytesWireSize(test_bytes2));
  Bytes test_bytes3(new Bytes::element_type(1024, '\1'));
  REQUIRE(4 + 1024 == BytesWireSize(test_bytes3));
}

TEST_CASE("RequestWriteTest.MessageWireSize")
{
  Message test_message;
  test_message.mutable_key().reset(new Bytes::element_type(1024));
  test_message.mutable_value().reset(new Bytes::element_type(2048));
  size_t expected_size =
    4 +  // Crc
      1 +  // Magic Byte
      1 +  // Attributes
      (4 + 1024) + // Key
      (4 + 2048);  // Value
  REQUIRE(expected_size == MessageWireSize(test_message));
}

TEST_CASE("RequestWriteTest.MessageSetWireSize")
{
  MessageAndOffset test_message;
  test_message.mutable_key().reset(new Bytes::element_type(1024));
  test_message.mutable_value().reset(new Bytes::element_type(2048));
  MessageSet test_message_set;
  test_message_set.push_back(test_message);
  test_message_set.push_back(test_message);
  test_message_set.push_back(test_message);
  // See test above
  size_t expected_message_size = MessageWireSize(test_message);
  // 3x (offset + msg_size + message)
  size_t expected_size = 3 * (8 + 4 + expected_message_size);
  REQUIRE(expected_size == MessageSetWireSize(test_message_set));
}

TEST_CASE("RequestWriteTest.RequestMessageWireSize")
{
  MessageAndOffset test_message;
  test_message.mutable_key().reset(new Bytes::element_type(1024));
  test_message.mutable_value().reset(new Bytes::element_type(2048));
  // See test above
  size_t expected_message_size = MessageWireSize(test_message);

  { // Metadata Request
    MetadataRequest req;
    req.AddTopicName("foo");
    req.AddTopicName("bar!");
    size_t expected_size =
      4 +        // Array size
        (2 + 3) +  // 'foo'
        (2 + 4);   // 'bar'
    REQUIRE(expected_size == RequestMessageWireSize(req));
  }
  { // Produce Request
    ProduceRequest req;
    req.AddMessage(test_message, "bar!");
    size_t expected_size =
      2 +  // RequiredAcks
        4 +  // Timeout
        4 +  // Topic Array Size
        (2 + 4) +  // 'bar!'
        4 +  // Partition Array Size
        4 +  // Partition
        4 +  // MessageSetSize
        (8 + 4 + expected_message_size);  // MessageSet
    REQUIRE(expected_size == RequestMessageWireSize(req));
  }
  { // Fetch Request
    FetchRequest req;
    req.FetchTopic("foo", 0);
    size_t expected_size =
      4 +  // ReplicaId
        4 +  // MaxWaitTime
        4 +  // MinBytes
        4 +  // Topic Array Size
        (2 + 3) +  // 'foo'
        4 +  // Partition Array Size
        4 +  // Partition
        8 +  // Fetch Offset
        4;   // MaxBytes
    REQUIRE(expected_size == RequestMessageWireSize(req));
  }
  { // Offset Request
    OffsetRequest req;
    req.FetchTopicOffset("foo", 0, 2500);
    size_t expected_size =
      4 +  // ReplicaId
        4 +  // Topic Array Size
        (2 + 3) +  // 'foo'
        4 +  // Partition Array Size
        4 +  // Partition
        8 +  // Time
        4;   // MaxNumberOfOffsets
    REQUIRE(expected_size== RequestMessageWireSize(req));
  }
}

TEST_CASE("RequestWriteTest.WriteInt8")
{
  {
		StreamTest s;
    WriteInt8(0, *s.stream);
    Bytes bytes = s.ReadEverything();
    REQUIRE(static_cast<bool>(bytes));
    REQUIRE(1== bytes->size());
    REQUIRE(0x00 == bytes->front());
  }
  {  // Big-endian
	  StreamTest s;
    WriteInt8(-1, *s.stream);
    Bytes bytes = s.ReadEverything();
    REQUIRE(static_cast<bool>(bytes));
    REQUIRE(1 == bytes->size());
    REQUIRE(0xff == bytes->front());
  }
}

TEST_CASE("RequestWriteTest.WriteInt16")
{
  {
		StreamTest s;
    WriteInt16(0, *s.stream);
    Bytes bytes = s.ReadEverything();
    REQUIRE(static_cast<bool>(bytes));
    REQUIRE(2 == bytes->size());
    REQUIRE(0x00 == bytes->at(0));
    REQUIRE(0x00 == bytes->at(1));
  }
  {
	  StreamTest s;
    WriteInt16(-1, *s.stream);
    Bytes bytes = s.ReadEverything();
    REQUIRE(static_cast<bool>(bytes));
    REQUIRE(2 == bytes->size());
    REQUIRE(0xff == bytes->at(0));
    REQUIRE(0xff == bytes->at(1));
  }
  {  // Big-endian
	  StreamTest s;
    WriteInt16(256, *s.stream);
    Bytes bytes = s.ReadEverything();
    REQUIRE(static_cast<bool>(bytes));
    REQUIRE(2 == bytes->size());
    REQUIRE(0x01 == bytes->at(0));
    REQUIRE(0x00 == bytes->at(1));
  }
}

TEST_CASE("RequestWriteTest.WriteInt32")
{
  {
		StreamTest s;
    WriteInt32(0, *s.stream);
    Bytes bytes = s.ReadEverything();
    REQUIRE(static_cast<bool>(bytes));
    REQUIRE(4 == bytes->size());
    REQUIRE(0x00 == bytes->at(0));
    REQUIRE(0x00 == bytes->at(1));
    REQUIRE(0x00 == bytes->at(2));
    REQUIRE(0x00 == bytes->at(3));
  }
  {
	  StreamTest s;
    WriteInt32(-1, *s.stream);
    Bytes bytes = s.ReadEverything();
    REQUIRE(static_cast<bool>(bytes));
    REQUIRE(4 == bytes->size());
    REQUIRE(0xff == bytes->at(0));
    REQUIRE(0xff == bytes->at(1));
    REQUIRE(0xff == bytes->at(2));
    REQUIRE(0xff == bytes->at(3));
  }
  {  // Big-endian
	  StreamTest s;
    WriteInt32(65536, *s.stream);
    Bytes bytes = s.ReadEverything();
    REQUIRE(static_cast<bool>(bytes));
    REQUIRE(4 == bytes->size());
    REQUIRE(0x00 == bytes->at(0));
    REQUIRE(0x01 == bytes->at(1));
    REQUIRE(0x00 == bytes->at(2));
    REQUIRE(0x00 == bytes->at(3));
  }
}

TEST_CASE("RequestWriteTest.WriteInt64")
{
  {
		StreamTest s;
    WriteInt64(0, *s.stream);
    Bytes bytes = s.ReadEverything();
    REQUIRE(static_cast<bool>(bytes));
    REQUIRE(8 == bytes->size());
    REQUIRE(0x00 == bytes->at(0));
    REQUIRE(0x00 == bytes->at(1));
    REQUIRE(0x00 == bytes->at(2));
    REQUIRE(0x00 == bytes->at(3));
    REQUIRE(0x00 == bytes->at(4));
    REQUIRE(0x00 == bytes->at(5));
    REQUIRE(0x00 == bytes->at(6));
    REQUIRE(0x00 == bytes->at(7));
  }
  {
	  StreamTest s;
    WriteInt64(-1, *s.stream);
    Bytes bytes = s.ReadEverything();
    REQUIRE(static_cast<bool>(bytes));
    REQUIRE(8 == bytes->size());
    REQUIRE(0xff == bytes->at(0));
    REQUIRE(0xff == bytes->at(1));
    REQUIRE(0xff == bytes->at(2));
    REQUIRE(0xff == bytes->at(3));
    REQUIRE(0xff == bytes->at(4));
    REQUIRE(0xff == bytes->at(5));
    REQUIRE(0xff == bytes->at(6));
    REQUIRE(0xff == bytes->at(7));
  }
  {  // Big-endian
	  StreamTest s;
    WriteInt64(4294967296, *s.stream);
    Bytes bytes = s.ReadEverything();
    REQUIRE(static_cast<bool>(bytes));
    REQUIRE(8 == bytes->size());
    REQUIRE(0x00 == bytes->at(0));
    REQUIRE(0x00 == bytes->at(1));
    REQUIRE(0x00 == bytes->at(2));
    REQUIRE(0x01 == bytes->at(3));
    REQUIRE(0x00 == bytes->at(4));
    REQUIRE(0x00 == bytes->at(5));
    REQUIRE(0x00 == bytes->at(6));
    REQUIRE(0x00 == bytes->at(7));
  }
}

TEST_CASE("RequestWriteTest.WriteString")
{
	StreamTest s;
  String test_string = "foo";
  WriteString(test_string, *s.stream);
  Bytes bytes = s.ReadEverything();
  REQUIRE(static_cast<bool>(bytes));
  REQUIRE(2 + 3 == bytes->size());
  REQUIRE(0 == bytes->at(0));
  REQUIRE(3 == bytes->at(1));
  REQUIRE('f' == bytes->at(2));
  REQUIRE('o' == bytes->at(3));
  REQUIRE('o' == bytes->at(4));
}

TEST_CASE("RequestWriteTest.WriteBytes")
{
	StreamTest s;
  Bytes test_bytes(new Bytes::element_type(3, 0xfe));
  WriteBytes(test_bytes, *s.stream);
  Bytes bytes = s.ReadEverything();
  REQUIRE(static_cast<bool>(bytes));
  REQUIRE(4 + 3 == bytes->size());
  REQUIRE(0 == bytes->at(0));
  REQUIRE(0 == bytes->at(1));
  REQUIRE(0 == bytes->at(2));
  REQUIRE(3 == bytes->at(3));
  REQUIRE(0xfe == bytes->at(4));
  REQUIRE(0xfe == bytes->at(5));
  REQUIRE(0xfe == bytes->at(6));
}
