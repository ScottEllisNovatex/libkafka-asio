//
// message_test.cpp
// ----------------
//
// Copyright (c) 2015 Daniel Joos
//
// Distributed under MIT license. (See file LICENSE)
//

#include <string>
#include <catch.hpp>
#include <libkafka_asio/libkafka_asio.h>

using libkafka_asio::Message;
using libkafka_asio::MessageAndOffset;
using libkafka_asio::MessageSet;
using libkafka_asio::CompressMessageSet;
using libkafka_asio::Bytes;

TEST_CASE("MessageTest.FlatCopy")
{
	Message orig;
	std::string test_value = "foo bar";
	orig.mutable_value().reset(
		new Bytes::element_type(test_value.begin(), test_value.end()));
	Message copy(orig, false);
	REQUIRE(static_cast<bool>(orig.value()));
	REQUIRE(static_cast<bool>(copy.value()));
	// Both messages should point to the same value buffer
	REQUIRE(orig.value().get() == copy.value().get());
}

TEST_CASE("MessageTest.AssignmentOperator")
{
	Message orig;
	std::string test_value = "foo bar";
	orig.mutable_value().reset(
		new Bytes::element_type(test_value.begin(), test_value.end()));
	Message copy;
	copy = orig;
	REQUIRE(static_cast<bool>(orig.value()));
	REQUIRE(static_cast<bool>(copy.value()));
	// Both messages should point to the same value buffer
	REQUIRE(orig.value().get() == copy.value().get());
}

TEST_CASE("MessageTest.DeepCopy")
{
	Message orig;
	{
		std::string test_value = "foo bar";
		orig.mutable_value().reset(
			new Bytes::element_type(test_value.begin(), test_value.end()));
	}
	Message copy(orig, true);
	REQUIRE(static_cast<bool>(orig.value()));
	REQUIRE(static_cast<bool>(copy.value()));
	// Each message should now have it's own value buffer
	REQUIRE_FALSE(orig.value().get() == copy.value().get());
	// But the actual value should be the same (a copy)
	REQUIRE_FALSE(orig.value()->empty());
	REQUIRE_FALSE(copy.value()->empty());
	std::string test_value1((const char*) & (*orig.value())[0],
		orig.value()->size());
	std::string test_value2((const char*) & (*copy.value())[0],
		copy.value()->size());
	REQUIRE(test_value1 == test_value2);
}

TEST_CASE("MessageTest.CompressMessageSet")
{
	MessageSet message_set(2);
	message_set[0].set_offset(1);
	message_set[1].set_offset(2);
	asio::error_code ec;
	using namespace libkafka_asio::constants;
	Message msg = CompressMessageSet(message_set, kCompressionGZIP, ec);
	REQUIRE(libkafka_asio::kErrorSuccess == ec);
	REQUIRE(static_cast<bool>(msg.value()));
	REQUIRE_FALSE(msg.value()->empty());
	REQUIRE(kCompressionGZIP == msg.compression());
}

TEST_CASE("MessageTest.CompressMessageSetNoneCompression")
{
	MessageSet message_set(2);
	message_set[0].set_offset(1);
	message_set[1].set_offset(2);
	asio::error_code ec;
	using namespace libkafka_asio::constants;
	Message msg = CompressMessageSet(message_set, kCompressionNone, ec);
	REQUIRE(libkafka_asio::kErrorCompressionFailed == ec);
}
