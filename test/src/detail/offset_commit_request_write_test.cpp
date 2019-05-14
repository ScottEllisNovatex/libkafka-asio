//
// detail/offset_commit_request_write_test.cpp
// -------------------------------------------
//
// Copyright (c) 2015 Daniel Joos
//
// Distributed under MIT license. (See file LICENSE)
//

#include <catch.hpp>
#include <libkafka_asio/libkafka_asio.h>
#include "StreamTest.h"

using libkafka_asio::OffsetCommitRequest;

TEST_CASE("OffsetCommitRequestWriteTest.WriteRequestMessage")
{
	StreamTest s;
	OffsetCommitRequest request;
	request.set_consumer_group("TestConsumerGroup");
	request.CommitOffset("Topic1", 4, 1234, 88888888, "my metadata");
	request.CommitOffset("Topic1", 5, 5678, 99999999);
	request.CommitOffset("Topic2", 0, 1234);

	libkafka_asio::detail::WriteRequestMessage(request, *s.stream);

	using namespace libkafka_asio::detail;
	using namespace libkafka_asio::constants;
	REQUIRE("TestConsumerGroup" == ReadString(*s.stream));
	REQUIRE(2 == ReadInt32(*s.stream));  // Topic array size

	REQUIRE("Topic1" == ReadString(*s.stream));  // TopicName
	REQUIRE(2 == ReadInt32(*s.stream));  // Partition array size
	REQUIRE(4 == ReadInt32(*s.stream));  // Partition
	REQUIRE(1234 == ReadInt64(*s.stream));  // Offset
	REQUIRE(88888888 == ReadInt64(*s.stream));  // Timestamp
	REQUIRE("my metadata" == ReadString(*s.stream));  // Metadata
	REQUIRE(5 == ReadInt32(*s.stream));  // Partition
	REQUIRE(5678 == ReadInt64(*s.stream));  // Offset
	REQUIRE(99999999 == ReadInt64(*s.stream));  // Timestamp
	REQUIRE("" == ReadString(*s.stream));  // Metadata

	REQUIRE("Topic2" == ReadString(*s.stream));  // TopicName
	REQUIRE(1 == ReadInt32(*s.stream));  // Partition array size
	REQUIRE(0 == ReadInt32(*s.stream));  // Partition
	REQUIRE(1234 == ReadInt64(*s.stream));  // Offset
	REQUIRE(kDefaultOffsetCommitTimestampNow == ReadInt64(*s.stream));  // Timestamp
	REQUIRE("" == ReadString(*s.stream));  // Metadata

	// Nothing else ...
	REQUIRE(0 == s.streambuf->size());
}

TEST_CASE("OffsetCommitRequestWriteTest.WriteRequestMessage_Empty")
{
	StreamTest s;
	OffsetCommitRequest request;
	libkafka_asio::detail::WriteRequestMessage(request, *s.stream);

	using namespace libkafka_asio::detail;
	REQUIRE("" == ReadString(*s.stream));  // ConsumerGroup
	REQUIRE(0 == ReadInt32(*s.stream));  // Topic array size

	// Nothing else ...
	REQUIRE(0 == s.streambuf->size());
}
