//
// detail/consumer_metadata_request_write_test.cpp
// -----------------------------------------------
//
// Copyright (c) 2015 Daniel Joos
//
// Distributed under MIT license. (See file LICENSE)
//

#include <catch.hpp>
#include <libkafka_asio/libkafka_asio.h>

#include "StreamTest.h"

using libkafka_asio::ConsumerMetadataRequest;

TEST_CASE("ConsumerMetadataRequestWriteTest.WriteRequestMessage")
{
  ConsumerMetadataRequest request;
  StreamTest s;
  request.set_consumer_group("TestConsumerGroup");
  libkafka_asio::detail::WriteRequestMessage(request, *s.stream);

  using namespace libkafka_asio::detail;
  REQUIRE("TestConsumerGroup" == ReadString(*s.stream));

  // Nothing else ...
  REQUIRE(0 == s.streambuf->size());
}

TEST_CASE("ConsumerMetadataRequestWriteTest.WriteRequestMessage_Empty")
{
  ConsumerMetadataRequest request;
  StreamTest s;
  libkafka_asio::detail::WriteRequestMessage(request, *s.stream);

  using namespace libkafka_asio::detail;
  REQUIRE("" == ReadString(*s.stream));

  // Nothing else ...
  REQUIRE(0 == s.streambuf->size());
}
