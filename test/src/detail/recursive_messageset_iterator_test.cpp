//
// detail/recursive_messageset_iterator_test.cpp
// ---------------------------------------------
//
// Copyright (c) 2015 Daniel Joos
//
// Distributed under MIT license. (See file LICENSE)
//

#include <algorithm>
#include <iterator>

#include <catch.hpp>

#include <libkafka_asio/detail/recursive_messageset_iterator.h>

using namespace libkafka_asio;
using namespace libkafka_asio::detail;

class MessageGenerator
{
public:
  MessageGenerator() :
    default_count_(0),
    count_(default_count_)
  {
  }

  MessageGenerator(Int64& count) :
    count_(count)
  {
  }

  MessageAndOffset operator()()
  {
    return MessageAndOffset(Message(), count_++);
  }

private:
  Int64 default_count_;
  Int64& count_;
};

TEST_CASE("RecursiveMessageSetIteratorTest.Empty")
{
  MessageSet empty_set;
  RecursiveMessageSetIterator iter(empty_set), end_iter;
  REQUIRE(0 == std::distance(iter, end_iter));
  REQUIRE(end_iter == iter);
  ++iter;
  REQUIRE(end_iter == iter);
}

TEST_CASE("RecursiveMessageSetIteratorTest.Flat")
{
  MessageSet flat_messages;
  std::generate_n(std::back_inserter(flat_messages), 10, MessageGenerator());
  REQUIRE(10 == std::distance(RecursiveMessageSetIterator(flat_messages),
                              RecursiveMessageSetIterator()));
  RecursiveMessageSetIterator iter(flat_messages), end_iter;
  Int64 count = 0;
  for (; iter != end_iter; ++iter)
  {
    REQUIRE(count == iter->offset());
    ++count;
  }
}

TEST_CASE("RecursiveMessageSetIteratorTest.Recurse")
{
  // Construct the following hierarchy:
  //
  // [0] (offset 0)
  //  |- [0] (offset 5) *
  //  |- [1] (offset 6) *
  // [1] (offset 1) *
  // [2] (offset 2)
  //  |- [0] (offset 7) *
  // [3] (offset 3)
  //  |- [0] (offset 8) *
  //  |- [1] (offset 9)
  //  |   |- [0] (offset 10) *
  //  |   |- [1] (offset 11) *
  //  |   |- [2] (offset 12) *
  // [4] (offset 4)
  //  |- [0] (offset 13) *
  //  |- [1] (offset 14) *
  //
  // All elements with star (*) should be visited by the iterator.
  //
  Int64 count = 0;
  MessageGenerator generator(count);
  MessageSet messages;
  std::generate_n(
    std::back_inserter(messages),
    5, generator);
  std::generate_n(
    std::back_inserter(messages[0].mutable_nested_message_set()),
    2, generator);
  std::generate_n(
    std::back_inserter(messages[2].mutable_nested_message_set()),
    1, generator);
  std::generate_n(
    std::back_inserter(messages[3].mutable_nested_message_set()),
    2, generator);
  std::generate_n(
    std::back_inserter(messages[3].mutable_nested_message_set()
                       [1].mutable_nested_message_set()),
    3, generator);
  std::generate_n(
    std::back_inserter(messages[4].mutable_nested_message_set()),
    2, generator);

  // The expected offset sequence:
  // (5, 6), 1, (7), (8, (10, 11, 12)), (13, 14)
  Int64 expected_offsets[] = {5, 6, 1, 7, 8, 10, 11, 12, 13, 14};
  RecursiveMessageSetIterator iter(messages), end_iter;
  for(Int64 expected_offset: expected_offsets)
  {
    REQUIRE(expected_offset == iter->offset());
    iter++;
  }
  REQUIRE(end_iter == iter);
}