# libkafka-asio
C++ Kafka Client Library using Boost Asio

[![Documentation Status](https://readthedocs.org/projects/libkafka-asio/badge/?version=latest)](https://readthedocs.org/projects/libkafka-asio/?badge=latest)

## Changes from Original  (danieljoos)
* Now depends on C++11 features
* Changed to use asio instead of boost::asio. This means asio is include file only, with no linking required.
* Tried to remove as much boost dependency as possible, by using C++11 features.
* Remaining boost dependancies are: crc, endian, iterator_facade. 
* Have included in submodules the boost files 1.70.0 that are required.
* Added submodules for zlib and snappy.
* Added appveyor configuration to do windows builds. This also builds the submodules, but in the configuration stage.
* Changed testing library from google test to catch2. (Again header only)
* Also added a runsettings file,so that the Catch test runner for Visual Studio will work. https://marketplace.visualstudio.com/items?itemName=JohnnyHendriks.ext01

Why all the changes? This is so that it fits better with our target project OpenDataCon.

## Introduction

`libkafka-asio` is a C++ header-only library, implementing the Kafka client 
protocol. All Kafka APIs, including offset commit/fetch, are implemented:

* [Metadata](https://cwiki.apache.org/confluence/display/KAFKA/A+Guide+To+The+Kafka+Protocol#AGuideToTheKafkaProtocol-MetadataAPI)
* [Produce](https://cwiki.apache.org/confluence/display/KAFKA/A+Guide+To+The+Kafka+Protocol#AGuideToTheKafkaProtocol-ProduceAPI)
* [Fetch](https://cwiki.apache.org/confluence/display/KAFKA/A+Guide+To+The+Kafka+Protocol#AGuideToTheKafkaProtocol-FetchAPI)
* [Offset](https://cwiki.apache.org/confluence/display/KAFKA/A+Guide+To+The+Kafka+Protocol#AGuideToTheKafkaProtocol-OffsetAPI)
* [Offset Commit/Fetch](https://cwiki.apache.org/confluence/display/KAFKA/A+Guide+To+The+Kafka+Protocol#AGuideToTheKafkaProtocol-OffsetCommit/FetchAPI)

It supports GZIP and Snappy message compression.

The library was tested on Linux (GCC 4.9, clang 3.5.1) and Windows (MSVC 10, 11, 12). There is a set of unit tests available under [`test`](test/).

## Usage

Add the [`lib`](lib/) directory to your include paths and:
```
#include <libkafka_asio/libkafka_asio.h>
```
Please see the [`examples`](examples/) directory for some examples on how to use the library.

Also consult the documentation: [libkafka-asio Reference](http://libkafka-asio.rtfd.org/)

## Dependencies

* asio - https://think-async.com/Asio
* zlib - https://github.com/madler/zlib
* snappy - https://github.com/google/snappy
* catch2 - https://github.com/catchorg/Catch2
* Boost 1.70.0 Dont need to install Boost, we have a reduced set in the repo
* spdlog - https://github.com/gabime/spdlog
