//
// connection_configuration_test.cpp
// -----------------------------
//
// Copyright (c) 2015 Daniel Joos
//
// Distributed under MIT license. (See file LICENSE)
//

#include <catch.hpp>
#include <libkafka_asio/libkafka_asio.h>
#include "../../lib/libkafka_asio/connection_configuration.h"

using namespace libkafka_asio;

typedef struct
{
	std::string host;
	unsigned int port;
}TestBrokerStruct;


TEST_CASE("ConnectionConfigurationTest.SetBrokerFromString_Empty")
{
	ConnectionConfiguration configuration;
	configuration.SetBrokerFromString("");
	REQUIRE_FALSE(configuration.broker_address);
}

TEST_CASE("ConnectionConfigurationTest.SetBrokerFromString")
{
	ConnectionConfiguration configuration;
	configuration.SetBrokerFromString("localhost:1234");
	REQUIRE(static_cast<bool>(configuration.broker_address));
	REQUIRE("localhost" == configuration.broker_address->hostname);
	REQUIRE("1234" == configuration.broker_address->service);
}

TEST_CASE("ConnectionConfigurationTest.SetBrokerFromString_NoService")
{
	ConnectionConfiguration configuration;
	configuration.SetBrokerFromString("localhost");
	REQUIRE(static_cast<bool>(configuration.broker_address));
	REQUIRE("localhost" == configuration.broker_address->hostname);
	// Default Kafka Service:
	REQUIRE("9092" == configuration.broker_address->service);
}

TEST_CASE("ConnectionConfigurationTest.SetBrokerFromString_Colon")
{
	ConnectionConfiguration configuration;
	configuration.SetBrokerFromString(":");
	REQUIRE_FALSE(static_cast<bool>(configuration.broker_address));
}

TEST_CASE("ConnectionConfigurationTest.SetBroker_1")
{
	ConnectionConfiguration configuration;
	TestBrokerStruct t;
	t.host = "example.org";
	t.port = 8888;
	configuration.SetBroker(t);
	REQUIRE(static_cast<bool>(configuration.broker_address));
	REQUIRE("example.org" == configuration.broker_address->hostname);
	REQUIRE("8888" == configuration.broker_address->service);
}

TEST_CASE("ConnectionConfigurationTest.SetBroker_2")
{
	ConnectionConfiguration configuration;
	configuration.SetBroker("example.org", 1234);
	REQUIRE(static_cast<bool>(configuration.broker_address));
	REQUIRE("example.org" == configuration.broker_address->hostname);
	REQUIRE("1234" == configuration.broker_address->service);
}
