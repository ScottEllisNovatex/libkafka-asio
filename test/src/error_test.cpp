//
// error_test.cpp
// --------------
//
// Copyright (c) 2015 Daniel Joos
//
// Distributed under MIT license. (See file LICENSE)
//

#include <gtest/gtest.h>
#include <libkafka_asio/libkafka_asio.h>

using namespace libkafka_asio;

TEST(ErrorTest, ClientErrorCategory)
{
  //using asio::error_code;
  asio::error_code error = make_error_code(kErrorAlreadyConnected);
  ASSERT_STREQ("libkafka_asio::ClientError", error.category().name());
  error = make_error_code(kErrorNotConnected);
  ASSERT_STREQ("libkafka_asio::ClientError", error.category().name());
  error = make_error_code(kErrorInProgress);
  ASSERT_STREQ("libkafka_asio::ClientError", error.category().name());
  error = make_error_code(kErrorNoBroker);
  ASSERT_STREQ("libkafka_asio::ClientError", error.category().name());
  // Boost system should be able to generate an error message:
  ASSERT_STREQ("No broker found", asio::system_error(error).what());
}

TEST(ErrorTest, KafkaErrorCategory)
{
  // 'Testing' two errors should suffice here
  asio::error_code error = make_error_code(kErrorNoError);
  ASSERT_STREQ("libkafka_asio::KafkaError", error.category().name());
  error = make_error_code(kErrorMessageSizeTooLarge);
  ASSERT_STREQ("libkafka_asio::KafkaError", error.category().name());
  // Boost system should be able to generate an error message:
  ASSERT_STREQ("Message was too large",
               asio::system_error(error).what());
}