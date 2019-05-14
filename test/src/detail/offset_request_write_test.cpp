//
// detail/offset_request_write_test.cpp
// ------------------------------------
//
// Copyright (c) 2015 Daniel Joos
//
// Distributed under MIT license. (See file LICENSE)
//

#include <catch.hpp>
#include <libkafka_asio/libkafka_asio.h>

#include "StreamTest.h"

using libkafka_asio::OffsetRequest;



TEST_CASE("OffsetRequestWriteTest.WriteRequestMessage")
{
	StreamTest s;
	OffsetRequest request;
	request.FetchTopicOffset("Topic1", 1);
	request.FetchTopicOffset("Topic2", 3, -2, 5);
	libkafka_asio::detail::WriteRequestMessage(request, *s.stream);

	using namespace libkafka_asio::detail;
	using namespace libkafka_asio::constants;
	REQUIRE(-1 == ReadInt32(*s.stream));  // ReplicaId
	REQUIRE(2 == ReadInt32(*s.stream));  // Topic array size

	REQUIRE("Topic1" == ReadString(*s.stream));  // TopicName
	REQUIRE(1 == ReadInt32(*s.stream));  // Partition array size
	REQUIRE(1 == ReadInt32(*s.stream));  // Partition
	REQUIRE(kOffsetTimeLatest == ReadInt64(*s.stream));  // Time
	REQUIRE(kDefaultOffsetMaxNumberOfOffsets == ReadInt32(*s.stream));

	REQUIRE("Topic2" == ReadString(*s.stream));  // TopicName
	REQUIRE(1 == ReadInt32(*s.stream));  // Partition array size
	REQUIRE(3 == ReadInt32(*s.stream));  // Partition
	REQUIRE(-2 == ReadInt64(*s.stream));  // Time
	REQUIRE(5 == ReadInt32(*s.stream));  // MaxNumberOfOffsets

	// Nothing else ...
	REQUIRE(0 == s.streambuf->size());
}

TEST_CASE("OffsetRequestWriteTest.WriteRequestMessage_Empty")
{
	StreamTest s;
	OffsetRequest request;
	libkafka_asio::detail::WriteRequestMessage(request, *s.stream);

	using namespace libkafka_asio::detail;
	using namespace libkafka_asio::constants;
	REQUIRE(-1 == ReadInt32(*s.stream));  // ReplicaId
	REQUIRE(0 == ReadInt32(*s.stream));  // Topic array size

	// Nothing else ...
	REQUIRE(0 == s.streambuf->size());
}