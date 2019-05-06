//
// examples/produce_cxx11.cpp
// --------------------------
//
// Copyright (c) 2015 Daniel Joos
//
// Distributed under MIT license. (See file LICENSE)
//
// ----------------------------------
//
// This example shows how to prepare a 'Produce' request, connect to a Kafka 
// server and send the produce request. Errors will be reported to stderr.
// The code uses lambda expressions, which were introduced in C++11.
// Therefore your compiler needs to know about C++11 and respective flags need
// to be set!
//

#include <iostream>
#include <boost/asio.hpp>
#include <libkafka_asio/libkafka_asio.h>

using libkafka_asio::Connection;
using libkafka_asio::ProduceRequest;
using libkafka_asio::ProduceResponse;

int main(int argc, char **argv)
{
  Connection::Configuration configuration;
  configuration.auto_connect = true;
  configuration.client_id = "libkafka_asio_example";
  configuration.socket_timeout = 10000;
  configuration.SetBrokerFromString("localhost:9092");


  boost::asio::io_service ios;
  Connection connection(ios, configuration);

  // Create a 'Produce' request and add a single message to it. The value of
  // that message is set to "Hello World". The message is produced for topic
  // "mytopic" and partition 0.
  ProduceRequest request;
  
  std::string MessageValue = "Hello World 2 Big Dogg";
  std::string MessageValue2 = "2 Big Dogg 22222";
  std::string MessageTopic = "Test";
  std::string MessageKey = "RTU12";

//#define ONEWAY
  request.set_required_acks(1);	// This is the default anyway. We can set to 0 so we dont need an ack, or more than 1 of there is more than one message in a message (I think)

#ifdef ONEWAY
  request.AddValue(MessageValue, MessageTopic, 0);
#else
 
  libkafka_asio::Message message;
  message.mutable_value().reset( new libkafka_asio::Bytes::element_type(MessageValue.begin(), MessageValue.end()));
  message.mutable_key().reset(new libkafka_asio::Bytes::element_type(MessageKey.begin(), MessageKey.end()));
  
  libkafka_asio::MessageAndOffset messageandoffset(message, 1);	// Offset seems to be 1 based??

  message.mutable_value().reset(new libkafka_asio::Bytes::element_type(MessageValue2.begin(), MessageValue2.end()));
  libkafka_asio::MessageAndOffset messageandoffset2(message, 2);	

  libkafka_asio::MessageSet messageset;
  messageset.push_back(messageandoffset);
  messageset.push_back(messageandoffset2);

  // Not sure if we have to do this or the library will if the flag is set...
  boost::system::error_code ec;
  libkafka_asio::Message smallmessage =  CompressMessageSet(messageset, libkafka_asio::constants::Compression::kCompressionSnappy, ec);
  if (libkafka_asio::kErrorSuccess != ec)
  {
	  // Compression failed...
	  std::cout << "Compression of message failed! " << ec << std::endl;
  }
  request.AddMessage(smallmessage, MessageTopic, 0);
#endif

  // Send the prepared produce request.
  // The connection will attempt to automatically connect to one of the brokers,
  // specified in the configuration.
  connection.AsyncRequest(
    request,
    [&](const Connection::ErrorCodeType& err,
        const ProduceResponse::OptionalType& response)
  {
    if (err)
    {
      std::cerr
        << "Error: " << boost::system::system_error(err).what()
        << std::endl;
      return;
    }
    std::cout << "Successfully produced message!" << std::endl;
  });

  // Let's go!
  ios.run();
  return 0;
}
