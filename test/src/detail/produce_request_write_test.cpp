//
// detail/produce_request_write_test.cpp
// -------------------------------------
//
// Copyright (c) 2015 Daniel Joos
//
// Distributed under MIT license. (See file LICENSE)
//

#include <catch.hpp>
#include <libkafka_asio/libkafka_asio.h>

#include "StreamTest.h"

using libkafka_asio::ProduceRequest;
using libkafka_asio::Int32;

TEST_CASE("ProduceRequestWriteTest.WriteRequestMessage")
{
	StreamTest s;
	ProduceRequest request;
	request.set_required_acks(1);
	request.set_timeout(100);
	request.AddValue("Foo Bar", "Topic1", 2);
	REQUIRE(1 == request.topics().size());
	REQUIRE(1 == request.topics()[0].partitions.size());

	libkafka_asio::detail::WriteRequestMessage(request, *s.stream);

	using namespace libkafka_asio::detail;
	REQUIRE(1 == ReadInt16(*s.stream));  // RequiredAcks
	REQUIRE(100 == ReadInt32(*s.stream));  // Timeout
	REQUIRE(1 == ReadInt32(*s.stream));  // Topic array size
	REQUIRE("Topic1" == ReadString(*s.stream));  // TopicName
	REQUIRE(1 == ReadInt32(*s.stream));  // Partition array size
	REQUIRE(2 == ReadInt32(*s.stream));  // Partition

	// 'MessageSetWireSize' and 'ReadMessageSet' are tested somewhere else, so
	// let's just assume they work correctly.
	Int32 expected_message_set_size
		= MessageSetWireSize(request.topics()[0].partitions[0].messages);
	REQUIRE(expected_message_set_size == ReadInt32(*s.stream));  // MessageSetSize
	libkafka_asio::MessageSet message_set;
	asio::error_code ec;
	ReadMessageSet(*s.stream, message_set, expected_message_set_size, ec);

	// Nothing else ...
	REQUIRE(0 == s.streambuf->size());
}

TEST_CASE("ProduceRequestWriteTest.WriteRequestMessage_Empty")
{
	StreamTest s;
	ProduceRequest request;

	libkafka_asio::detail::WriteRequestMessage(request, *s.stream);

	using namespace libkafka_asio::detail;
	using namespace libkafka_asio::constants;
	REQUIRE(kDefaultProduceRequiredAcks == ReadInt16(*s.stream));  // RequiredAcks
	REQUIRE(kDefaultProduceTimeout == ReadInt32(*s.stream));  // Timeout
	REQUIRE(0 == ReadInt32(*s.stream));  // Topic array size

	// Nothing else ...
	REQUIRE(0 == s.streambuf->size());
}
