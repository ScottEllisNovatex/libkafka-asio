# Examples

Here you can find some examples showing how to use `libkafka-asio`.
c++03 Examples removed.

   
The following examples are available in the above folders:

- [`fetch.cpp`]
  Use a fetch request to get messages for a topic partition.
- [`metadata.cpp`]
  Get topic metadata to determine leading brokers.
- [`offset.cpp`]
  Get the current offset number of a topic partition.
- [`offset_fetch.cpp`]
  Shows how to get offset data for a topic in a consumer group. Also 
  illustrates the use of futures and promises.
- [`produce.cpp`]
  Produce a _Hello World_ message on a topic partition.