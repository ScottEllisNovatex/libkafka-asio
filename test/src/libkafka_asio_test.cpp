//
// libkafka_asio_test.cpp
// ----------------------
//
// Copyright (c) 2015 Daniel Joos
//
// Distributed under MIT license. (See file LICENSE)
//

#define CATCH_CONFIG_RUNNER
#include <catch.hpp>

TEST_CASE("Root.Catch2Test")
{
	INFO("Message From Catch2");
	REQUIRE(true);

}
int main(int argc, char* argv[])
{
	// Create loggers for tests here
	//CommandLineLoggingSetup();

	return Catch::Session().run(argc, argv);
	// And release here.
//	CommandLineLoggingCleanup();
}
