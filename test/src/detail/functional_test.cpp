//
// detail/functional_test.cpp
// --------------------------
//
// Copyright (c) 2015 Daniel Joos
//
// Distributed under MIT license. (See file LICENSE)
//

#include <vector>
#include <catch.hpp>
#include <libkafka_asio/libkafka_asio.h>

using namespace libkafka_asio;
using namespace libkafka_asio::detail;


class TestTopic
{
public:
	String topic_name;

	TestTopic()
	{
	}

	TestTopic(const String& name) : topic_name(name)
	{
	}
};
class TestPartition
{
public:
	Int32 partition;

	TestPartition()
	{
	}

	TestPartition(Int32 p) :
		partition(p)
	{
	}
};
 
typedef std::vector<TestTopic> TestTopicVector;
typedef std::vector<TestPartition> TestPartitionVector;

TEST_CASE("FunctionalTest.FindTopicByNameConst")
{
	TestTopicVector test_topics;
	test_topics.push_back(TestTopic("hello"));
	test_topics.push_back(TestTopic("world"));
	test_topics.push_back(TestTopic("foo"));
	test_topics.push_back(TestTopic("bar"));
	REQUIRE(4 == test_topics.size());
	const TestTopicVector & const_test_topics = test_topics;
	TestTopicVector::const_iterator result;
	result = FindTopicByName("foo", const_test_topics);
	REQUIRE(result != const_test_topics.end());
	result = FindTopicByName("test", const_test_topics);
	REQUIRE(result == const_test_topics.end());
	REQUIRE(4 == test_topics.size());
}

TEST_CASE("FunctionalTest.FindTopicByName")
{
	TestTopicVector test_topics;
	test_topics.push_back(TestTopic("hello"));
	test_topics.push_back(TestTopic("world"));
	test_topics.push_back(TestTopic("foo"));
	test_topics.push_back(TestTopic("bar"));
	REQUIRE(4 == test_topics.size());
	TestTopicVector::iterator result;
	result = FindTopicByName("foo", test_topics);
	REQUIRE(result != test_topics.end());
	result = FindTopicByName("test", test_topics);
	REQUIRE(result != test_topics.end());
	result = FindTopicByName("yetanother", test_topics, false);
	REQUIRE(result == test_topics.end());
	REQUIRE(5 == test_topics.size());
}

TEST_CASE("FunctionalTest.FindTopicPartitionByNumberConst")
{
	TestPartitionVector test_partitions;
	test_partitions.push_back(TestPartition(1337));
	test_partitions.push_back(TestPartition(42));
	test_partitions.push_back(TestPartition(1));
	test_partitions.push_back(TestPartition(2));
	REQUIRE(4 == test_partitions.size());
	const TestPartitionVector & const_test_partitions = test_partitions;
	TestPartitionVector::const_iterator result;
	result = FindTopicPartitionByNumber(2, const_test_partitions);
	REQUIRE(result != const_test_partitions.end());
	result = FindTopicPartitionByNumber(9000, const_test_partitions);
	REQUIRE(result == const_test_partitions.end());
	REQUIRE(4 == test_partitions.size());
}

TEST_CASE("FunctionalTest.FindTopicPartitionByNumber")
{
	TestPartitionVector test_partitions;
	test_partitions.push_back(TestPartition(1337));
	test_partitions.push_back(TestPartition(42));
	test_partitions.push_back(TestPartition(1));
	test_partitions.push_back(TestPartition(2));
	REQUIRE(4 == test_partitions.size());
	TestPartitionVector::iterator result;
	result = FindTopicPartitionByNumber(2, test_partitions);
	REQUIRE(result != test_partitions.end());
	result = FindTopicPartitionByNumber(9000, test_partitions);
	REQUIRE(result != test_partitions.end());
	result = FindTopicPartitionByNumber(1234, test_partitions, false);
	REQUIRE(result == test_partitions.end());
	REQUIRE(5 == test_partitions.size());
}