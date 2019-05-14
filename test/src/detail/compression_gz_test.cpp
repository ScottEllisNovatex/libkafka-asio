//
// detail/compression_gz_test.cpp
// ------------------------------
//
// Copyright (c) 2015 Daniel Joos
//
// Distributed under MIT license. (See file LICENSE)
//
#include <catch.hpp>
#include <libkafka_asio/libkafka_asio.h>

using libkafka_asio::Byte;
using libkafka_asio::Bytes;
using libkafka_asio::String;
using libkafka_asio::detail::Compress;
using libkafka_asio::detail::Decompress;

class CompressionGzTest
{
public:
	static Bytes TestCase1Compressed()
	{
		const unsigned char data[32] =
		{
		  0x1f, 0x8b, 0x08, 0x08, 0x75, 0xad, 0x3b, 0x55,
		  0x00, 0x03, 0x74, 0x65, 0x73, 0x74, 0x00, 0x4b,
		  0xcb, 0xcf, 0x4f, 0x4a, 0x2c, 0xe2, 0x02, 0x00,
		  0x47, 0x97, 0x2c, 0xb2, 0x07, 0x00, 0x00, 0x00
		};
		return Bytes(new Bytes::element_type(data, data + sizeof(data)));
	}

	static Bytes TestCase1Uncompressed()
	{
		static const String data = "foobar\n";
		return Bytes(new Bytes::element_type(data.begin(), data.end()));
	}

	static void SkipGzipHeader(Bytes & data)
	{
		::z_stream gz = {};
		::gz_header header = {};
		int ret = ::inflateInit2(&gz, 15 + 32);
		gz.next_in = reinterpret_cast<::Bytef*>(&(*data)[0]);
		gz.avail_in = (::uInt) data->size();
		ret = ::inflateGetHeader(&gz, &header);
		while (ret == Z_OK && !header.done)
		{
			Byte buf;
			gz.next_out = reinterpret_cast<::Bytef*>(&buf);
			gz.avail_out = 1;
			ret = ::inflate(&gz, Z_BLOCK);
		}
		::inflateEnd(&gz);
		size_t skip = gz.total_in;
		data->erase(data->begin(), data->begin() + skip);
	}
};

TEST_CASE("GZIP.SimpleDecompress")
{
	asio::error_code ec;
	using namespace libkafka_asio::constants;
	Bytes result = Decompress(CompressionGzTest::TestCase1Compressed(), kCompressionGZIP, ec);
	Bytes expected_result = CompressionGzTest::TestCase1Uncompressed();
	REQUIRE(libkafka_asio::kErrorSuccess == ec);
	REQUIRE(*expected_result == *result);
}

TEST_CASE("GZIP.EmptyDecompress")
{
	{
		Bytes test_data;
		asio::error_code ec;
		using namespace libkafka_asio::constants;
		Bytes result = Decompress(test_data, kCompressionGZIP, ec);
		REQUIRE(libkafka_asio::kErrorCompressionFailed == ec);
		REQUIRE(!result);
	}
	{
		Bytes test_data(new Bytes::element_type());
		asio::error_code ec;
		REQUIRE(0 == test_data->size());
		using namespace libkafka_asio::constants;
		Bytes result = Decompress(test_data, kCompressionGZIP, ec);
		REQUIRE(libkafka_asio::kErrorCompressionFailed == ec);
		REQUIRE(!result);
	}
}

TEST_CASE("GZIP.SimpleCompress")
{
	asio::error_code ec;
	using namespace libkafka_asio::constants;
	Bytes result = Compress(CompressionGzTest::TestCase1Uncompressed(), kCompressionGZIP, ec);
	Bytes expected_result = CompressionGzTest::TestCase1Compressed();
	REQUIRE(libkafka_asio::kErrorSuccess == ec);
	CompressionGzTest::SkipGzipHeader(result);
	CompressionGzTest::SkipGzipHeader(expected_result);
	REQUIRE(*expected_result == *result);
}

TEST_CASE("GZIP.EmptyCompress")
{
	{
		Bytes test_data;
		asio::error_code ec;
		using namespace libkafka_asio::constants;
		Bytes result = Compress(test_data, kCompressionGZIP, ec);
		REQUIRE(libkafka_asio::kErrorCompressionFailed == ec);
		REQUIRE(!result);
	}
	{
		Bytes test_data(new Bytes::element_type());
		asio::error_code ec;
		REQUIRE(0 == test_data->size());
		using namespace libkafka_asio::constants;
		Bytes result = Compress(test_data, kCompressionGZIP, ec);
		REQUIRE(libkafka_asio::kErrorCompressionFailed == ec);
		REQUIRE(!result);
	}
}
