//
// detail/impl/connection_service.h
// --------------------------------
//
// Copyright (c) 2015 Daniel Joos
//
// Distributed under MIT license. (See file LICENSE)
//

#ifndef CONNECTION_SERVICE_H_5310FB3D_9D78_4C52_AE32_EB71E000F4ED
#define CONNECTION_SERVICE_H_5310FB3D_9D78_4C52_AE32_EB71E000F4ED


#include <libkafka_asio/constants.h>
#include <libkafka_asio/detail/request_write.h>
#include <libkafka_asio/detail/response_read.h>
#include <libkafka_asio/error.h>
#include <libkafka_asio/detail/weak_impl_handler.h>

namespace libkafka_asio
{
namespace detail
{

template<typename Service>
asio::io_service::id BasicConnectionService<Service>::id;

inline ConnectionServiceImpl::ConnectionServiceImpl(
  asio::io_service& io_service) :
  connection_state_(kConnectionStateClosed),
  write_state_(kTxStateIdle),
  read_state_(kTxStateIdle),
  io_service_(io_service),
  socket_(io_service),
  connect_deadline_(io_service),
  write_deadline_(io_service),
  read_deadline_(io_service),
  resolver_(io_service)
{
}

inline const ConnectionConfiguration&
ConnectionServiceImpl::configuration() const
{
  return configuration_;
}

inline void ConnectionServiceImpl::set_configuration(
  const ConnectionConfiguration& configuration)
{
  configuration_ = configuration;
}

inline void ConnectionServiceImpl::Close()
{
  connection_state_ = kConnectionStateClosed;
  asio::error_code ec;
  resolver_.cancel();
  socket_.shutdown(SocketType::shutdown_both, ec);
  socket_.close(ec);
  connect_deadline_.cancel();
  write_deadline_.cancel();
  read_deadline_.cancel();
}

inline void ConnectionServiceImpl::AsyncConnect(
  const std::string& host,
  const std::string& service,
  const ConnectionServiceImpl::ConnectionHandlerType& handler)
{
  if (connection_state_ != kConnectionStateClosed)
  {
    io_service_.post(std::bind(handler, kErrorAlreadyConnected));
    return;
  }
  ResolverType::query query(host, service);
  resolver_.async_resolve(
    query,
    WeakImpl<ConnectionServiceImpl>::ResolveHandler(
      shared_from_this(),
      std::bind(
        &ConnectionServiceImpl::HandleAsyncResolve, this,
        std::placeholders::_1,
		  std::placeholders::_2,
        handler)));
  connection_state_ = kConnectionStateConnecting;
  SetDeadline(connect_deadline_);
}

inline void ConnectionServiceImpl::AsyncConnect(
  const ConnectionServiceImpl::ConnectionHandlerType& handler)
{
  if (!configuration_.broker_address)
  {
    io_service_.post(std::bind(handler, kErrorNoBroker));
    return;
  }
  AsyncConnect(
      configuration_.broker_address->hostname,
      configuration_.broker_address->service,
      handler);
}

template<typename TRequest>
inline void ConnectionServiceImpl::AsyncRequest(
  const TRequest& request,
  const typename ConnectionServiceImpl::Handler<TRequest>::Type& handler)
{
  io_service_.post(
    WeakImpl<ConnectionServiceImpl>::NullaryHandler(
      shared_from_this(),
      std::bind(
        &ConnectionServiceImpl::EnqueueRequest<TRequest>, this,
        request,
        handler)));
}

inline void ConnectionServiceImpl::SetDeadline(
  ConnectionServiceImpl::DeadlineTimerType& timer)
{
  using std::chrono::milliseconds;
  timer.expires_from_now(milliseconds(configuration_.socket_timeout));
  timer.async_wait(
    WeakImpl<ConnectionServiceImpl>::DeadlineHandler(
      shared_from_this(),
      std::bind(
        &ConnectionServiceImpl::HandleDeadline, this,
        std::placeholders::_1,
        std::ref(timer))));
}

template<typename TRequest>
inline ConnectionServiceImpl::StreambufType ConnectionServiceImpl::Serialize(
  const TRequest& request)
{
  StreambufType buffer(new StreambufType::element_type());
  std::ostream os(buffer.get());
  detail::WriteRequest(request, configuration_.client_id, os);
  return buffer;
}

template<typename TRequest>
inline void ConnectionServiceImpl::EnqueueRequest(
  const TRequest& request,
  const typename ConnectionServiceImpl::Handler<TRequest>::Type& handler)
{
  if (connection_state_ == kConnectionStateClosed)
  {
    if (!configuration_.auto_connect)
    {
      typename TRequest::ResponseType::OptionalType empty_response;
      io_service_.post(
        std::bind(handler, kErrorNotConnected, empty_response));
      return;
    }
    else if (!configuration_.broker_address)
    {
      typename TRequest::ResponseType::OptionalType empty_response;
      io_service_.post(
        std::bind(handler, kErrorNoBroker, empty_response));
      return;
    }
  }
  QueueItem item;
  item.buffer = Serialize(request);
  bool response_expected = request.ResponseExpected();
  item.handler =
    std::bind(
      &ConnectionServiceImpl::HandleAsyncRequestWrite<TRequest>, this,
      std::placeholders::_1,
      std::placeholders::_2,
      item.buffer,
      handler,
      response_expected);
  if (write_state_ == kTxStateIdle)
  {
    if (write_queue_.empty() &&
        connection_state_ == kConnectionStateConnected)
    {
      SendRequest(item);
    }
    else
    {
      write_queue_.push_back(item);
      NextRequest();
    }
  }
  else
  {
    write_queue_.push_back(item);
  }
}

template<typename TRequest>
inline void ConnectionServiceImpl::EnqueueResponse(
  ConnectionServiceImpl::StreambufType buffer,
  const typename ConnectionServiceImpl::Handler<TRequest>::Type& handler)
{
  if (connection_state_ != kConnectionStateConnected)
  {
    typename TRequest::ResponseType::OptionalType empty_response;
    io_service_.post(std::bind(handler, kErrorNotConnected, empty_response));
    while (!read_queue_.empty())
    {
      QueueItem& item = read_queue_.front();
      io_service_.post(std::bind(item.handler, kErrorNotConnected, 0));
      read_queue_.pop_front();
    }
    NextRequest();
    return;
  }
  QueueItem item;
  item.buffer = buffer;
  item.handler =
    std::bind(
      &ConnectionServiceImpl::HandleAsyncResponseSizeRead<TRequest>, this,
      std::placeholders::_1,
      std::placeholders::_2,
      buffer,
      handler);
  if (read_state_ == kTxStateIdle)
  {
    if (read_queue_.empty())
    {
      ReceiveResponse(item);
    }
    else
    {
      read_queue_.push_back(item);
      NextResponse();
    }
  }
  else
  {
    read_queue_.push_back(item);
  }
}

inline void ConnectionServiceImpl::NextRequest()
{
  if (write_state_ == kTxStateBusy || write_queue_.empty())
  {
    return;
  }
  if (connection_state_ == kConnectionStateClosed &&
      configuration_.auto_connect)
  {
    AsyncConnect(
      WeakImpl<ConnectionServiceImpl>::ErrorHandler(
        shared_from_this(),
        std::bind(&ConnectionServiceImpl::HandleAsyncAutoConnect, this,
                    std::placeholders::_1)));
    return;
  }
  if (connection_state_ == kConnectionStateConnected)
  {
    QueueItem& item = write_queue_.front();
    SendRequest(item);
    write_queue_.pop_front();
  }
}

inline void ConnectionServiceImpl::NextResponse()
{
  if (read_state_ == kTxStateBusy || read_queue_.empty())
  {
    return;
  }
  QueueItem& item = read_queue_.front();
  ReceiveResponse(item);
  read_queue_.pop_front();
}

inline void ConnectionServiceImpl::SendRequest(
  const ConnectionServiceImpl::QueueItem& item)
{
  asio::async_write(
    socket_, *item.buffer,
    WeakImpl<ConnectionServiceImpl>::WriteHandler(
      shared_from_this(),
      item.handler));
  write_state_ = kTxStateBusy;
  SetDeadline(write_deadline_);
}

inline void ConnectionServiceImpl::ReceiveResponse(
  const ConnectionServiceImpl::QueueItem& item)
{
  asio::async_read(
    socket_,
    item.buffer->prepare(sizeof(Int32)),
    asio::transfer_exactly(sizeof(Int32)),
    WeakImpl<ConnectionServiceImpl>::ReadHandler(
      shared_from_this(),
      item.handler));
  read_state_ = kTxStateBusy;
  SetDeadline(read_deadline_);
}

inline void ConnectionServiceImpl::HandleAsyncResolve(
  const ConnectionServiceImpl::ErrorCodeType& error,
  const ConnectionServiceImpl::ResolverType::iterator& iter,
  const ConnectionServiceImpl::ConnectionHandlerType& handler)
{
  if (error)
  {
    io_service_.post(std::bind(handler, error));
    Close();
    return;
  }
  asio::async_connect(
    socket_, iter,
    WeakImpl<ConnectionServiceImpl>::ConnectHandler(
      shared_from_this(),
      std::bind(
        &ConnectionServiceImpl::HandleAsyncConnect, this,
        std::placeholders::_1,
        std::placeholders::_2,
        handler)));
  SetDeadline(connect_deadline_);
}

inline void ConnectionServiceImpl::HandleAsyncConnect(
  const ConnectionServiceImpl::ErrorCodeType& error,
  const ConnectionServiceImpl::ResolverType::iterator&,
  const ConnectionServiceImpl::ConnectionHandlerType& handler)
{
  if (error)
  {
    Close();
  }
  else
  {
    connection_state_ = kConnectionStateConnected;
    connect_deadline_.cancel();
  }
  io_service_.post(std::bind(handler, error));
}

inline void ConnectionServiceImpl::HandleAsyncAutoConnect(
  const ConnectionServiceImpl::ErrorCodeType& error)
{
  if (error)
  {
    while (!write_queue_.empty())
    {
      QueueItem& item = write_queue_.front();
      io_service_.post(std::bind(item.handler, error, 0));
      write_queue_.pop_front();
    }
    return;
  }
  if (write_queue_.empty())
  {
    return;
  }
  QueueItem& item = write_queue_.front();
  SendRequest(item);
  write_queue_.pop_front();
}

template<typename TRequest>
inline void ConnectionServiceImpl::HandleAsyncRequestWrite(
  const ConnectionServiceImpl::ErrorCodeType& error,
  size_t bytes_transferred,
  ConnectionServiceImpl::StreambufType buffer,
  const typename ConnectionServiceImpl::Handler<TRequest>::Type& handler,
  bool response_expected)
{
  // parameter not used
  (void)bytes_transferred;
  write_state_ = kTxStateIdle;
  write_deadline_.cancel();
  typedef typename TRequest::ResponseType::OptionalType OptionalResponse;
  if (error)
  {
    OptionalResponse empty_response;
    io_service_.post(std::bind(handler, error, empty_response));
    Close();
    return;
  }
  if (!response_expected)
  {
    OptionalResponse empty_response;
    io_service_.post(std::bind(handler, error, empty_response));
    NextRequest();
    return;
  }
  buffer->consume(buffer->size());
  EnqueueResponse<TRequest>(buffer, handler);
}

template<typename TRequest>
inline void ConnectionServiceImpl::HandleAsyncResponseSizeRead(
  const ConnectionServiceImpl::ErrorCodeType& error,
  size_t bytes_transferred,
  ConnectionServiceImpl::StreambufType buffer,
  const typename ConnectionServiceImpl::Handler<TRequest>::Type& handler)
{
  typedef typename TRequest::ResponseType::OptionalType OptionalResponse;
  if (error)
  {
    OptionalResponse empty_response;
    io_service_.post(std::bind(handler, error, empty_response));
    Close();
    return;
  }
  buffer->commit(bytes_transferred);
  std::istream is(buffer.get());
  Int32 size = detail::ReadInt32(is);
  if (size >= configuration_.message_max_bytes)
  {
    OptionalResponse empty_response;
    io_service_.post(std::bind(handler, kErrorMessageSizeTooLarge,
                                 empty_response));
    Close();
    return;
  }
  asio::async_read(
    socket_,
    buffer->prepare(size),
    asio::transfer_exactly(size),
    WeakImpl<ConnectionServiceImpl>::ReadHandler(
      shared_from_this(),
      std::bind(
        &ConnectionServiceImpl::HandleAsyncResponseRead<TRequest>, this,
        std::placeholders::_1,
        std::placeholders::_2,
        buffer,
        handler)));
  SetDeadline(read_deadline_);
}

template<typename TRequest>
inline void ConnectionServiceImpl::HandleAsyncResponseRead(
  const ConnectionServiceImpl::ErrorCodeType& error,
  size_t bytes_transferred,
  ConnectionServiceImpl::StreambufType buffer,
  const typename ConnectionServiceImpl::Handler<TRequest>::Type& handler)
{
  typedef typename TRequest::ResponseType::OptionalType OptionalResponse;
  if (error)
  {
    OptionalResponse empty_response;
    io_service_.post(std::bind(handler, error, empty_response));
    Close();
    return;
  }
  read_state_ = kTxStateIdle;
  read_deadline_.cancel();
  buffer->commit(bytes_transferred);
  std::istream is(buffer.get());
  typename TRequest::MutableResponseType response;
  asio::error_code ec;
  detail::ReadResponse(is, response, ec);
  if (ec)
  {
    OptionalResponse empty_response;
    io_service_.post(std::bind(handler, ec, empty_response));
  }
  else
  {
    io_service_.post(std::bind(handler, ec, response.response()));
  }
  NextRequest();
}

inline void ConnectionServiceImpl::HandleDeadline(
  const ConnectionServiceImpl::ErrorCodeType& error,
  ConnectionServiceImpl::DeadlineTimerType& timer)
{
  if (error)
  {
    return;
  }
  if (timer.expires_at() <= std::chrono::system_clock::now())
  {
    connect_deadline_.expires_at(std::chrono::system_clock::time_point::max());
    write_deadline_.expires_at(std::chrono::system_clock::time_point::max());
    read_deadline_.expires_at(std::chrono::system_clock::time_point::max());
    Close();
  }
}

}  // namespace detail
}  // namespace libkafka_asio

#endif  // CONNECTION_SERVICE_H_5310FB3D_9D78_4C52_AE32_EB71E000F4ED
