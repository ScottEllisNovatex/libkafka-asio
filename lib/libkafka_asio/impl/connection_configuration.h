#ifndef CONNECTION_CONFIGURATION_H_0435D34D_97AC_4D67_8E3A_DEBAAC218C4D
#define CONNECTION_CONFIGURATION_H_0435D34D_97AC_4D67_8E3A_DEBAAC218C4D

#include <libkafka_asio/constants.h>

namespace libkafka_asio
{

inline ConnectionConfiguration::ConnectionConfiguration() :
  message_max_bytes(constants::kDefaultMessageMaxBytes),
  socket_timeout(constants::kDefaultSocketTimeout),
  client_id(constants::DefaultClientId()),
  auto_connect(constants::DefaultConnectionAutoConnect())
{
}

inline void ConnectionConfiguration::SetBrokerFromString(const std::string& str)
{
  if (str.empty())
  {
    broker_address.reset();
    return;
  }
  BrokerAddress broker;
  std::string::size_type delimiter_position = str.find(':');
  if (delimiter_position != std::string::npos &&
      delimiter_position > 0 &&
      delimiter_position < str.size() - 1)
  {
    broker.hostname = str.substr(0, delimiter_position);
    broker.service = str.substr(delimiter_position + 1);
  }
  else
  {
    broker.hostname = str.substr(0, delimiter_position);
    broker.service = constants::DefaultKafkaService();
  }
  if (broker.hostname.empty() || broker.service.empty())
  {
    return;
  }
  broker_address = broker;
}

template<typename T>
inline void ConnectionConfiguration::SetBroker(const T& broker)
{
  SetBroker(broker.host, broker.port);
}

inline void ConnectionConfiguration::SetBroker(const std::string & hostname, const uint32_t & service)
{
  BrokerAddress broker;
  broker.hostname = hostname;
  broker.service = std::to_string(service);
  broker_address = broker;
}
inline void ConnectionConfiguration::SetBroker(const std::string& hostname,	const std::string & service)
{
	BrokerAddress broker;
	broker.hostname = hostname;
	broker.service = service;
	broker_address = broker;
}

}  // namespace libkafka_asio

#endif  // CONNECTION_CONFIGURATION_H_0435D34D_97AC_4D67_8E3A_DEBAAC218C4D
