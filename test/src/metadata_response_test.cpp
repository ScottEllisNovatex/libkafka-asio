//
// metadata_response_test.cpp
// --------------------------
//
// Copyright (c) 2015 Daniel Joos
//
// Distributed under MIT license. (See file LICENSE)
//

#include <catch.hpp>
#include <libkafka_asio/libkafka_asio.h>

using namespace libkafka_asio;

class MetadataResponseTest 
{
public:
  void AddBroker(const String& host, Int32 node_id, Int32 port)
  {
    MetadataResponse::Broker broker;
    broker.host = host;
    broker.node_id = node_id;
    broker.port = port;
	MetadataResponseTest::response.mutable_brokers().push_back(broker);
  }

  MutableMetadataResponse response;
};


TEST_CASE("MetadataResponseTest.PartitionLeader")
{
	MetadataResponseTest mrt;
	mrt.AddBroker("localhost", 123, 49152);
	mrt.AddBroker("example.com", 456, 49152);
	REQUIRE(2 == mrt.response.response().brokers().size());
	MetadataResponse::Topic metadata;
	MetadataResponse::Partition test_partition;
	test_partition.leader = 456;
	metadata.partitions.insert(std::make_pair(1, test_partition));
	mrt.response.mutable_topics().insert(std::make_pair("foo", metadata));
	REQUIRE(1 == mrt.response.response().topics().size());

	MetadataResponse::Broker::OptionalType leader = mrt.response.response().PartitionLeader("foo", 1);
	REQUIRE(static_cast<bool>(leader));
	REQUIRE(456 == leader->node_id);
	REQUIRE("example.com" == leader->host);
}

TEST_CASE("MetadataResponseTest.PartitionLeader_InElection")
{
	MetadataResponse::Topic metadata;
	MetadataResponse::Partition test_partition;
	MetadataResponseTest mrt;
	test_partition.leader = -1;
	metadata.partitions.insert(std::make_pair(1, test_partition));
	mrt.response.mutable_topics().insert(std::make_pair("foo", metadata));
	REQUIRE(1 == mrt.response.response().topics().size());

	MetadataResponse::Broker::OptionalType leader = mrt.response.response().PartitionLeader("foo", 1);
	REQUIRE_FALSE(static_cast<bool>(leader));
}
