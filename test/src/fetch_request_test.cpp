//
// fetch_request_test.cpp
// ----------------------
//
// Copyright (c) 2015 Daniel Joos
//
// Distributed under MIT license. (See file LICENSE)
//

#include <catch.hpp>
#include <libkafka_asio/libkafka_asio.h>

TEST_CASE("FetchRequestTest.FetchTopic_New")
{
	libkafka_asio::FetchRequest request;
	REQUIRE(0 == request.topics().size());
	request.FetchTopic("mytopic", 1, 2);
	REQUIRE(1 == request.topics().size());
	REQUIRE(1 == request.topics()[0].partitions.size());
	REQUIRE("mytopic" == request.topics()[0].topic_name);
	REQUIRE(1 == request.topics()[0].partitions[0].partition);
	REQUIRE(2 == request.topics()[0].partitions[0].fetch_offset);
	REQUIRE(libkafka_asio::constants::kDefaultFetchMaxBytes == request.topics()[0].partitions[0].max_bytes);
}

TEST_CASE("FetchRequestTest.FetchTopic_Override")
{
	libkafka_asio::FetchRequest request;
	REQUIRE(0 == request.topics().size());
	request.FetchTopic("mytopic", 1, 2);
	REQUIRE(1 == request.topics().size());
	REQUIRE(1 == request.topics()[0].partitions.size());
	REQUIRE(2 == request.topics()[0].partitions[0].fetch_offset);
	request.FetchTopic("mytopic", 1, 4);
	REQUIRE(1 == request.topics().size());
	REQUIRE(1 == request.topics()[0].partitions.size());
	REQUIRE(4 == request.topics()[0].partitions[0].fetch_offset);
}

TEST_CASE("FetchRequestTest.FetchTopic_MultiplePartitions")
{
	libkafka_asio::FetchRequest request;
	REQUIRE(0 == request.topics().size());
	request.FetchTopic("mytopic", 0, 2);
	request.FetchTopic("mytopic", 1, 4);
	REQUIRE(1 == request.topics().size());
	REQUIRE(2 == request.topics()[0].partitions.size());
	REQUIRE(2 == request.topics()[0].partitions[0].fetch_offset);
	REQUIRE(4 == request.topics()[0].partitions[1].fetch_offset);
}

TEST_CASE("FetchRequestTest.FetchTopic_MultipleTopics")
{
	libkafka_asio::FetchRequest request;
	REQUIRE(0 == request.topics().size());
	request.FetchTopic("foo", 0, 2);
	request.FetchTopic("bar", 1, 4);
	REQUIRE(2 == request.topics().size());
	REQUIRE(1 == request.topics()[0].partitions.size());
	REQUIRE(1 == request.topics()[1].partitions.size());
	REQUIRE(2 == request.topics()[0].partitions[0].fetch_offset);
	REQUIRE(4 == request.topics()[1].partitions[0].fetch_offset);
}
