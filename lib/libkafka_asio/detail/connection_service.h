//
// detail/connection_service.h
// ---------------------------
//
// Copyright (c) 2015 Daniel Joos
//
// Distributed under MIT license. (See file LICENSE)
//

#ifndef CONNECTION_SERVICE_H_BD87CD55_1B15_4F19_AE8C_B78C42BBE950
#define CONNECTION_SERVICE_H_BD87CD55_1B15_4F19_AE8C_B78C42BBE950

#include <deque>
#include <memory>
#include <asio.hpp>
#include <libkafka_asio/connection_configuration.h>

namespace libkafka_asio
{
namespace detail
{

template<typename Service>
class BasicConnectionService :
	public asio::io_service::service
{
public:

	// Service implementation type
	typedef std::shared_ptr<Service> implementation_type;

	// Unique service identifier
	static asio::io_service::id id;

	// Construct a new connection service
	explicit BasicConnectionService(asio::io_service& io_service) :
		asio::io_service::service(io_service)
	{
	}

	// Destroy all user-defined handler objects owned by the service
	void shutdown_service()
	{
	}

	// Construct a new connection service implementation
	void construct(implementation_type& impl)
	{
#if (ASIO_VERSION > 101100)
		impl.reset(new Service(get_io_context()));	// Was get_io_servce() get_io_context() SJE change to get Boost 1.70 / asio 1.12.2 working
#else
		impl.reset(new Service(get_io_service()));
#endif
	}

	// Destroy a connection service implementation
	void destroy(implementation_type& impl)
	{
		impl.reset();
	}
};

//
// Connection Service Implementation Class
//
// This class actually holds all objects, required for a single connection,
// such as the actual socket object as well as the write/read queues.
// It holds three different states: 1) connection state, 2) write operation
// state and 3) the read operation state. It manages a write and read queue
// in parallel, so there will always be one write and one read operation
// pending on the socket.
//
class ConnectionServiceImpl :
  public std::enable_shared_from_this<ConnectionServiceImpl>
{
public:
  // Error codes use this type
  typedef asio::error_code ErrorCodeType;

  // Handler type definition for connection operations
  typedef std::function<void(const ErrorCodeType&)> ConnectionHandlerType;

  // Handler type definitions for transmission operations
  typedef std::function<void(const ErrorCodeType&, size_t)> TxHandlerType;

  // Handler type deduction template depending on the type of request
  template<typename TRequest>
  struct Handler
  {
    typedef typename TRequest::ResponseType::OptionalType ResponseType;
    typedef std::function<void(const ErrorCodeType&,
                                 const ResponseType&)> Type;
  };

private:
  typedef asio::ip::tcp::socket SocketType;
  typedef asio::basic_waitable_timer<std::chrono::system_clock> TimerType;
  typedef asio::ip::tcp::resolver ResolverType;
  typedef std::shared_ptr<asio::streambuf> StreambufType;

  enum ConnectionState
  {
    kConnectionStateClosed = 0,
    kConnectionStateConnecting,
    kConnectionStateConnected
  };

  enum TxState
  {
    kTxStateIdle = 0,
    kTxStateBusy
  };

  struct QueueItem
  {
    StreambufType buffer;
    TxHandlerType handler;
  };

  typedef std::deque<QueueItem> WriteQueue;
  typedef std::deque<QueueItem> ReadQueue;

public:

  // Construct a new connection service object
  ConnectionServiceImpl(asio::io_service& io_service);

  // Gets the connection configuration
  const ConnectionConfiguration& configuration() const;

  // Sets the connection configuration
  void set_configuration(const ConnectionConfiguration& configuration);

  // Closes the connection to the Kafka server.
  void Close();

  // Asynchronous connection to the specified Kafka server
  void AsyncConnect(const std::string& host,
                    const std::string& service,
                    const ConnectionHandlerType& handler);

  // Asynchronous connection to the Kafka server, specified in the configuration
  void AsyncConnect(const ConnectionHandlerType& handler);

  // Asynchronously sends the given request to the connected Kafka server.
  template<typename TRequest>
  void AsyncRequest(const TRequest& request,
                    const typename Handler<TRequest>::Type& handler);

// Internal helper functions:
private:

  // Resets the operation timeout
  void SetDeadline(TimerType& timer);

  // Serialize the given request to the Kafka wire format
  template<typename TRequest>
  StreambufType Serialize(const TRequest& request);

// Flow functions:
private:

  // Push the request + handler to the write queue
  template<typename TRequest>
  void EnqueueRequest(const TRequest& request,
                      const typename Handler<TRequest>::Type& handler);

  // Push the given handler to the read queue
  template<typename TRequest>
  void EnqueueResponse(StreambufType buffer,
                       const typename Handler<TRequest>::Type& handler);

  // Take the next request from the write queue
  void NextRequest();

  // Work on the next response from the read queue
  void NextResponse();

  // Schedule the send operation for the given write queue item
  void SendRequest(const QueueItem& item);

  // Schedule the receive operation for the given read queue item
  void ReceiveResponse(const QueueItem& item);

// Handler functions:
private:

  // Handle async resolve operation
  void HandleAsyncResolve(const ErrorCodeType& error,
                          const ResolverType::iterator& iter,
                          const ConnectionHandlerType& handler);

  // Handle async connect operations
  void HandleAsyncConnect(const ErrorCodeType& error,
                          const ResolverType::iterator& iter,
                          const ConnectionHandlerType& handler);

  // Handle async auto-connect operations
  void HandleAsyncAutoConnect(const ErrorCodeType& error);

  // Handle async request write operations
  template<typename TRequest>
  void HandleAsyncRequestWrite(
    const ErrorCodeType& error,
    size_t bytes_transferred,
    StreambufType buffer,
    const typename Handler<TRequest>::Type& handler,
    bool response_expected);

  // Handle async read of response size
  template<typename TRequest>
  void HandleAsyncResponseSizeRead(
    const ErrorCodeType& error,
    size_t bytes_transferred,
    StreambufType buffer,
    const typename Handler<TRequest>::Type& handler);

  // Handle async read of response body
  template<typename TRequest>
  void HandleAsyncResponseRead(
    const ErrorCodeType& error,
    size_t bytes_transferred,
    StreambufType buffer,
    const typename Handler<TRequest>::Type& handler);

  // Handle operation timeout
  void HandleDeadline(const ErrorCodeType& error,
                      TimerType& timer);

private:
  ConnectionConfiguration configuration_;
  WriteQueue write_queue_;
  ReadQueue read_queue_;
  ConnectionState connection_state_;
  TxState write_state_;
  TxState read_state_;

  asio::io_service& io_service_;
  SocketType socket_;
  TimerType connect_deadline_;
  TimerType write_deadline_;
  TimerType read_deadline_;
  ResolverType resolver_;
};

typedef BasicConnectionService<ConnectionServiceImpl> ConnectionService;

}  // namespace detail
}  // namespace libkafka_asio

#include <libkafka_asio/detail/impl/connection_service.h>

#endif  // CONNECTION_SERVICE_H_BD87CD55_1B15_4F19_AE8C_B78C42BBE950
