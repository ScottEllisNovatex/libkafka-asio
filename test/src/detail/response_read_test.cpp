//
// detail/response_read_test.cpp
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

TEST_CASE("ResponseReadTest.ReadInt8")
{
	StreamTest s;
	Bytes bytes(new Bytes::element_type(1, 64));
	s.stream->write((const char*) & (*bytes)[0], bytes->size());
	REQUIRE(64 == ReadInt8(*s.stream));
	// Read again on exhausted stream:
	REQUIRE(0 == ReadInt8(*s.stream));
	REQUIRE_FALSE(s.stream->good());
	REQUIRE(0 == ReadInt8(*s.stream));
	REQUIRE(0 == ReadInt8(*s.stream));
}

TEST_CASE("ResponseReadTest.ReadInt16")
{
	StreamTest s;
	Bytes bytes(new Bytes::element_type());
	bytes->push_back(0x01);
	bytes->push_back(0x00);
	s.stream->write((const char*) & (*bytes)[0], bytes->size());
	REQUIRE(256 == ReadInt16(*s.stream));
}

TEST_CASE("ResponseReadTest.ReadInt32")
{
	StreamTest s;
	Bytes bytes(new Bytes::element_type());
	bytes->push_back(0x00);
	bytes->push_back(0x01);
	bytes->push_back(0x00);
	bytes->push_back(0x00);
	s.stream->write((const char*) & (*bytes)[0], bytes->size());
	REQUIRE(65536 == ReadInt32(*s.stream));
}

TEST_CASE("ResponseReadTest.ReadInt64")
{
	StreamTest s;
	Bytes bytes(new Bytes::element_type());
	bytes->push_back(0x00);
	bytes->push_back(0x00);
	bytes->push_back(0x00);
	bytes->push_back(0x01);
	bytes->push_back(0x00);
	bytes->push_back(0x00);
	bytes->push_back(0x00);
	bytes->push_back(0x00);
	s.stream->write((const char*) & (*bytes)[0], bytes->size());
	REQUIRE(4294967296 == ReadInt64(*s.stream));
}

TEST_CASE("ResponseReadTest.ReadString")
{
	StreamTest s;
	Bytes bytes(new Bytes::element_type());
	bytes->push_back(0);
	bytes->push_back(3);
	bytes->push_back('f');
	bytes->push_back('o');
	bytes->push_back('o');
	s.stream->write((const char*) & (*bytes)[0], bytes->size());
	REQUIRE("foo" == ReadString(*s.stream));
}

TEST_CASE("ResponseReadTest.ReadBytes")
{
	StreamTest s;
	Bytes bytes(new Bytes::element_type());
	bytes->push_back(0);
	bytes->push_back(0);
	bytes->push_back(0);
	bytes->push_back(2);
	bytes->push_back(0x01);
	bytes->push_back(0x02);
	s.stream->write((const char*) & (*bytes)[0], bytes->size());
	Bytes read_bytes;
	ReadBytes(*s.stream, read_bytes);
	REQUIRE(static_cast<bool>(read_bytes));
	REQUIRE(2 == read_bytes->size());
	REQUIRE(0x01 == read_bytes->at(0));
	REQUIRE(0x02 == read_bytes->at(1));
}

TEST_CASE("ResponseReadTest.ReadMessage")
{
	// Extracted this from `Wireshark`, capturing a `kafka-console-producer`
	// request with message value `helloworld`.
	const unsigned char test_data[24] =
	{
		0x6f, 0xff, 0xbb, 0x60, 0x00, 0x00, 0xff, 0xff,
		0xff, 0xff, 0x00, 0x00, 0x00, 0x0a, 0x68, 0x65,
		0x6c, 0x6c, 0x6f, 0x77, 0x6f, 0x72, 0x6c, 0x64
	};
	StreamTest s;
	s.stream->write((const char*)test_data, sizeof(test_data));
	Message message;
	asio::error_code ec;
	ReadMessage(*s.stream, message, ec);
	REQUIRE(kErrorSuccess == ec);
	REQUIRE_FALSE(static_cast<bool>(message.key()));
	REQUIRE(static_cast<bool>(message.value()));
	std::string message_value((const char*) & (*message.value())[0], message.value()->size());
	REQUIRE("helloworld" == message_value);
}

TEST_CASE("ResponseReadTest.ReadMessageSet")
{
	// MessageSet that contains 2 messages: `foo` and `bar`
	const unsigned char test_data[58] =
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x11, 0xfa, 0x8b, 0x1b, 0x4c,
		0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,
		0x00, 0x03, 0x66, 0x6f, 0x6f, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x11, 0x00, 0x07, 0xf2, 0xc7, 0x00, 0x00, 0xff,
		0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x03, 0x62,
		0x61, 0x72
	};
	StreamTest s;
	s.stream->write((const char*)test_data, sizeof(test_data));
	MessageSet message_set;
	asio::error_code ec;
	ReadMessageSet(*s.stream, message_set, sizeof(test_data), ec);
	REQUIRE(kErrorSuccess == ec);
	REQUIRE(2 == message_set.size());
	REQUIRE(static_cast<bool>(message_set[0].value()));
	REQUIRE(static_cast<bool>(message_set[1].value()));
	std::string message_value_0((const char*) & (*message_set[0].value())[0], message_set[0].value()->size());
	std::string message_value_1((const char*) & (*message_set[1].value())[0], message_set[1].value()->size());
	REQUIRE("foo" == message_value_0);
	REQUIRE("bar" == message_value_1);
}
