//
// error_test.cpp
// --------------
//
// Copyright (c) 2015 Daniel Joos
//
// Distributed under MIT license. (See file LICENSE)
//

#include <catch.hpp>
#include <libkafka_asio/libkafka_asio.h>

using namespace libkafka_asio;

TEST_CASE("ErrorTest.ClientErrorCategory")
{
  using asio::error_code;
  error_code error = error_code(kErrorAlreadyConnected);
  REQUIRE("libkafka_asio::ClientError" == std::string(error.category().name()));
  error = error_code(kErrorNotConnected);
  REQUIRE("libkafka_asio::ClientError" == std::string(error.category().name()));
  error = error_code(kErrorInProgress);
  REQUIRE("libkafka_asio::ClientError" == std::string(error.category().name()));
  error = error_code(kErrorNoBroker);
  REQUIRE("libkafka_asio::ClientError" == std::string(error.category().name()));
  // Asio system should be able to generate an error message:
  REQUIRE("No broker found" == std::string(asio::system_error(error).what()));
}

TEST_CASE("ErrorTest.KafkaErrorCategory")
{
  using asio::error_code;
  // 'Testing' two errors should suffice here
  error_code error = error_code(kErrorNoError);
  REQUIRE("libkafka_asio::KafkaError" == std::string(error.category().name()));
  error = error_code(kErrorMessageSizeTooLarge);
  REQUIRE("libkafka_asio::KafkaError" == std::string(error.category().name()));
  //Asio system should be able to generate an error message:
  REQUIRE("Message was too large" == std::string(asio::system_error(error).what()));
}