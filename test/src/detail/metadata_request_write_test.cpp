//
// detail/metadata_request_write_test.cpp
// --------------------------------------
//
// Copyright (c) 2015 Daniel Joos
//
// Distributed under MIT license. (See file LICENSE)
//

#include <catch.hpp>
#include <libkafka_asio/libkafka_asio.h>

#include "StreamTest.h"

using libkafka_asio::MetadataRequest;

TEST_CASE("MetadataRequestWriteTest.WriteRequestMessage")
{
	StreamTest s;
	MetadataRequest request;
	request.AddTopicName("Foo");
	request.AddTopicName("Bar");
	libkafka_asio::detail::WriteRequestMessage(request, *s.stream);

	using namespace libkafka_asio::detail;
	REQUIRE(2 == ReadInt32(*s.stream));  // Topic array size
	REQUIRE("Foo" == ReadString(*s.stream));
	REQUIRE("Bar" == ReadString(*s.stream));

	// Nothing else ...
	REQUIRE(0 == s.streambuf->size());
}

TEST_CASE("MetadataRequestWriteTest.WriteRequestMessage_Empt")
{
	StreamTest s;
	MetadataRequest request;
	libkafka_asio::detail::WriteRequestMessage(request, *s.stream);

	using namespace libkafka_asio::detail;
	REQUIRE(0 == ReadInt32(*s.stream));  // Topic array size

	// Nothing else ...
	REQUIRE(0 == s.streambuf->size());
}
