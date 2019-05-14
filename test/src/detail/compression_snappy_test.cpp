//
// detail/compression_snappy_test.cpp
// ----------------------------------
//
// Copyright (c) 2015 Daniel Joos
//
// Distributed under MIT license. (See file LICENSE)
//

#include <catch.hpp>
#include <snappy.h>
#include <libkafka_asio/libkafka_asio.h>

using libkafka_asio::Byte;
using libkafka_asio::Bytes;
using libkafka_asio::String;
using libkafka_asio::detail::Compress;
using libkafka_asio::detail::Decompress;

class CompressionSnappyTest
{
public:
	static Bytes TestCase1Compressed()
	{
		static const unsigned char data[11] =
		{
		  0x24, 0x14, 0x52, 0x45, 0x50, 0x45, 0x41, 0x54,
		  0x76, 0x06, 0x00
		};
		return Bytes(new Bytes::element_type(data, data + sizeof(data)));
	}

	static Bytes TestCase2Compressed()
	{
		static const unsigned char data[39] =
		{
		  0x82, 0x53, 0x4e, 0x41, 0x50, 0x50, 0x59, 0x00,
		  0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
		  0x00, 0x00, 0x00, 0x07, 0x05, 0x10, 0x48, 0x65,
		  0x6c, 0x6c, 0x6f, 0x00, 0x00, 0x00, 0x08, 0x06,
		  0x14, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64
		};
		return Bytes(new Bytes::element_type(data, data + sizeof(data)));
	}

	static Bytes TestCase1Uncompressed()
	{
		static const String data = "REPEATREPEATREPEATREPEATREPEATREPEAT";
		return Bytes(new Bytes::element_type(data.begin(), data.end()));
	}

	static Bytes TestCase2Uncompressed()
	{
		static const String data = "Hello World";
		return Bytes(new Bytes::element_type(data.begin(), data.end()));
	}
};

TEST_CASE("Snappy.SimpleDecompress")
{
	asio::error_code ec;
	using namespace libkafka_asio::constants;
	Bytes result = Decompress(CompressionSnappyTest::TestCase1Compressed(), kCompressionSnappy ,ec);
	Bytes expected_result = CompressionSnappyTest::TestCase1Uncompressed();
	REQUIRE(libkafka_asio::kErrorSuccess == ec);
	REQUIRE(*expected_result == *result);
}

TEST_CASE("Snappy.StreamDecompress")
{
	asio::error_code ec;
	using namespace libkafka_asio::constants;
	Bytes result = Decompress(CompressionSnappyTest::TestCase2Compressed(), kCompressionSnappy,ec);
	Bytes expected_result = CompressionSnappyTest::TestCase2Uncompressed();
	REQUIRE(libkafka_asio::kErrorSuccess == ec);
	REQUIRE(*expected_result == *result);
}

TEST_CASE("Snappy.EmptyDecompress")
{
	{
		Bytes test_data;
		asio::error_code ec;
		using namespace libkafka_asio::constants;
		Bytes result = Decompress(test_data, kCompressionSnappy,ec);
		REQUIRE(libkafka_asio::kErrorCompressionFailed == ec);
		REQUIRE(!result);
	}
	{
		Bytes test_data(new Bytes::element_type());
		asio::error_code ec;
		using namespace libkafka_asio::constants;
		Bytes result = Decompress(test_data, kCompressionSnappy,ec);
		REQUIRE(libkafka_asio::kErrorCompressionFailed == ec);
		REQUIRE(!result);
	}
}

TEST_CASE("Snappy.SimpleCompress")
{
	asio::error_code ec;
	using namespace libkafka_asio::constants;
	Bytes result = Compress(CompressionSnappyTest::TestCase1Uncompressed(), kCompressionSnappy,ec);
	Bytes expected_result = CompressionSnappyTest::TestCase1Compressed();
	REQUIRE(libkafka_asio::kErrorSuccess == ec);
	REQUIRE(*expected_result == *result);
}

TEST_CASE("Snappy.EmptyCompress")
{
	{
		Bytes test_data;
		asio::error_code ec;
		using namespace libkafka_asio::constants;
		Bytes result = Compress(test_data, kCompressionSnappy,ec);
		REQUIRE(libkafka_asio::kErrorCompressionFailed == ec);
		REQUIRE(!result);
	}
	{
		Bytes test_data(new Bytes::element_type());
		asio::error_code ec;
		using namespace libkafka_asio::constants;
		Bytes result = Compress(test_data, kCompressionSnappy,ec);
		REQUIRE(libkafka_asio::kErrorCompressionFailed == ec);
		REQUIRE(!result);
	}
}
