//
// detail/offset_fetch_request_write_test.cpp
// ------------------------------------------
//
// Copyright (c) 2015 Daniel Joos
//
// Distributed under MIT license. (See file LICENSE)
//

#include <catch.hpp>
#include <libkafka_asio/libkafka_asio.h>
#include "StreamTest.h"

using libkafka_asio::OffsetFetchRequest;


TEST_CASE("OffsetFetchRequestWriteTest.WriteRequestMessage")
{
	StreamTest s;
	OffsetFetchRequest request;
	request.set_consumer_group("TestGroup");
	request.FetchOffset("Topic1", 0);
	request.FetchOffset("Topic1", 1);
	request.FetchOffset("Topic2", 1);
	libkafka_asio::detail::WriteRequestMessage(request, *s.stream);

	using namespace libkafka_asio::detail;
	REQUIRE("TestGroup" == ReadString(*s.stream));  // ConsumerGroup
	REQUIRE(2 == ReadInt32(*s.stream));  // Topic array size
	REQUIRE("Topic1" == ReadString(*s.stream));  // TopicName
	REQUIRE(2 == ReadInt32(*s.stream));  // Partition array size
	REQUIRE(0 == ReadInt32(*s.stream));  // Partition
	REQUIRE(1 == ReadInt32(*s.stream));  // Partition
	REQUIRE("Topic2" == ReadString(*s.stream));  // TopicName
	REQUIRE(1 == ReadInt32(*s.stream));  // Partition array size
	REQUIRE(1 == ReadInt32(*s.stream));  // Partition

	// Nothing else ...
	REQUIRE(0 == s.streambuf->size());
}

TEST_CASE("OffsetFetchRequestWriteTest.WriteRequestMessage_Empty")
{
	StreamTest s;
	OffsetFetchRequest request;
	libkafka_asio::detail::WriteRequestMessage(request, *s.stream);

	using namespace libkafka_asio::detail;
	REQUIRE("" == ReadString(*s.stream));  // ConsumerGroup
	REQUIRE(0 == ReadInt32(*s.stream));  // Topic array size

	// Nothing else ...
	REQUIRE(0 == s.streambuf->size());
}
