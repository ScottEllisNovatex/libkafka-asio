//
// detail/fetch_request_write_test.cpp
// -----------------------------------
//
// Copyright (c) 2015 Daniel Joos
//
// Distributed under MIT license. (See file LICENSE)
//

#include <catch.hpp>
#include <libkafka_asio/libkafka_asio.h>

#include "StreamTest.h"

using libkafka_asio::FetchRequest;

TEST_CASE("FetchRequestWriteTest.WriteRequestMessage")
{
	FetchRequest request;
	StreamTest s;
	request.set_max_wait_time(100);
	request.set_min_bytes(1);
	request.FetchTopic("Topic1", 0, 123);
	request.FetchTopic("Topic2", 1, 456, 1024);

	libkafka_asio::detail::WriteRequestMessage(request, *s.stream);

	using namespace libkafka_asio::detail;
	REQUIRE(-1 == ReadInt32(*s.stream));  // ReplicaId
	REQUIRE(100 == ReadInt32(*s.stream));  // MaxWaitTime
	REQUIRE(1 == ReadInt32(*s.stream));  // MinBytes
	REQUIRE(2 == ReadInt32(*s.stream));  // Topic array size

	REQUIRE("Topic1" == ReadString(*s.stream));  // TopicName
	REQUIRE(1 == ReadInt32(*s.stream));  // Partition array size
	REQUIRE(0 == ReadInt32(*s.stream));  // Partition 0
	REQUIRE(123 == ReadInt64(*s.stream));  // FetchOffset 123
	REQUIRE(libkafka_asio::constants::kDefaultFetchMaxBytes == ReadInt32(*s.stream));  // MaxBytes (default)

	REQUIRE("Topic2" == ReadString(*s.stream));  // TopicName
	REQUIRE(1 == ReadInt32(*s.stream));  // Partition array size
	REQUIRE(1 == ReadInt32(*s.stream));  // Partition 1
	REQUIRE(456 == ReadInt64(*s.stream));  // FetchOffset 456
	REQUIRE(1024 == ReadInt32(*s.stream));  // MaxBytes 1024

	// Nothing else ...
	REQUIRE(0 == s.streambuf->size());
}

TEST_CASE("FetchRequestWriteTest.WriteRequestMessage_Empty")
{
	FetchRequest request;
	StreamTest s;
	libkafka_asio::detail::WriteRequestMessage(request, *s.stream);

	using namespace libkafka_asio::detail;
	using namespace libkafka_asio::constants;
	REQUIRE(-1 == ReadInt32(*s.stream));  // ReplicaId
	REQUIRE(kDefaultFetchMaxWaitTime == ReadInt32(*s.stream));  // MaxWaitTime
	REQUIRE(kDefaultFetchMinBytes == ReadInt32(*s.stream));  // MinBytes
	REQUIRE(0 == ReadInt32(*s.stream));  // Topic array size

	// Nothing else ...
	REQUIRE(0 == s.streambuf->size());
}
