// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

module;
#include "fabrictransport_.h"
#include <winrt/base.h>

#ifdef SF_DEBUG
#include <boost/log/trivial.hpp>
#endif

#include "servicefabric/async_context.hpp"
#include "servicefabric/waitable_callback.hpp"

#include "servicefabric/transport_dummy_client_conn_handler.hpp"
#include "servicefabric/transport_dummy_client_notification_handler.hpp"
#include "servicefabric/transport_dummy_msg_disposer.hpp"
#include "servicefabric/transport_dummy_server_conn_handler.hpp"
#include "servicefabric/transport_message.hpp"

#include <string>

export module kvtransport;

namespace sf = servicefabric;

// request handler
// TODO: implement logic
class request_handler
    : public winrt::implements<request_handler,
                               IFabricTransportMessageHandler> {
public:
  HRESULT STDMETHODCALLTYPE BeginProcessRequest(
      /* [in] */ COMMUNICATION_CLIENT_ID clientId,
      /* [in] */ IFabricTransportMessage *message,
      /* [in] */ DWORD timeoutMilliseconds,
      /* [in] */ IFabricAsyncOperationCallback *callback,
      /* [retval][out] */ IFabricAsyncOperationContext **context) override {
    UNREFERENCED_PARAMETER(clientId);
    UNREFERENCED_PARAMETER(timeoutMilliseconds);
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug)
        << "request_handler::BeginProcessRequest id: " << clientId;
#endif
    std::string body = sf::get_body(message);
    std::string headers = sf::get_header(message);
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug)
        << "request_handler::BeginProcessRequest header: " << headers
        << " body: " << body;
#endif

    winrt::com_ptr<IFabricAsyncOperationContext> ctx =
        winrt::make<sf::async_context>(callback);
    *context = ctx.detach();
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE EndProcessRequest(
      /* [in] */ IFabricAsyncOperationContext *context,
      /* [retval][out] */ IFabricTransportMessage **reply) override {
    UNREFERENCED_PARAMETER(context);
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug) << "request_handler::EndProcessRequest";
#endif
    winrt::com_ptr<IFabricTransportMessage> msg1 =
        winrt::make<sf::transport_message>("mybodyreply", "myheaderreply");
    *reply = msg1.detach();
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE HandleOneWay(
      /* [in] */ COMMUNICATION_CLIENT_ID clientId,
      /* [in] */ IFabricTransportMessage *message) override {
    UNREFERENCED_PARAMETER(clientId);
    UNREFERENCED_PARAMETER(message);
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug) << "request_handler::HandleOneWay";
#endif
    return S_OK;
  }
};

export class kv_server {
public:
  kv_server(std::wstring hostname, ULONG port)
      : req_handler_(winrt::make<request_handler>()),
        conn_handler_(winrt::make<sf::transport_dummy_server_conn_handler>()),
        msg_disposer_(winrt::make<sf::transport_dummy_msg_disposer>()),
        listener_(), hostname_(hostname), port_(port), listening_addr_(),
        is_listening_(false) {}

  // returns the listening address
  HRESULT open() {
    FABRIC_SECURITY_CREDENTIALS cred = {};
    cred.Kind = FABRIC_SECURITY_CREDENTIAL_KIND_NONE;

    FABRIC_TRANSPORT_SETTINGS settings = {};
    settings.KeepAliveTimeoutInSeconds = 10;
    settings.MaxConcurrentCalls = 10;
    settings.MaxMessageSize = 100;
    settings.MaxQueueSize = 100;
    settings.OperationTimeoutInSeconds = 30;
    settings.SecurityCredentials = &cred;

    FABRIC_TRANSPORT_LISTEN_ADDRESS addr = {};
    addr.IPAddressOrFQDN = hostname_.c_str();
    addr.Path = L"/";
    addr.Port = port_;

    // create listener
    HRESULT hr = CreateFabricTransportListener(
        IID_IFabricTransportListener, &settings, &addr, req_handler_.get(),
        conn_handler_.get(), msg_disposer_.get(), listener_.put());

    if (hr != S_OK) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(error)
          << "cannot CreateFabricTransportListener: " << hr;
#endif
      return hr;
    }

    // open listener
    winrt::com_ptr<sf::IFabricAsyncOperationWaitableCallback> callback =
        winrt::make<sf::FabricAsyncOperationWaitableCallback>();
    winrt::com_ptr<IFabricAsyncOperationContext> ctx;
    hr = listener_->BeginOpen(callback.get(), ctx.put());
    if (hr != S_OK) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(error) << "cannot BeginOpen: " << hr;
#endif
      return hr;
    }
    callback->Wait();
    winrt::com_ptr<IFabricStringResult> addr_str;
    hr = listener_->EndOpen(ctx.get(), addr_str.put());
    if (hr != S_OK) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(error) << "cannot EndOpen: " << hr;
#endif
      return hr;
    }
    this->listening_addr_ = std::wstring(addr_str->get_String());
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug) << "Listening on address: " << listening_addr_;
#endif
    this->is_listening_ = true;
    return S_OK;
  }

  std::wstring get_listening_addr() { return this->listening_addr_; }

  // can call multiple times
  HRESULT close() {
    if (!this->is_listening_) {
      return S_OK;
    }
    winrt::com_ptr<sf::IFabricAsyncOperationWaitableCallback> callback =
        winrt::make<sf::FabricAsyncOperationWaitableCallback>();
    winrt::com_ptr<IFabricAsyncOperationContext> ctx;
    HRESULT hr;
    hr = listener_->BeginClose(callback.get(), ctx.put());

    if (hr != S_OK) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(error) << "cannot BeginClose: " << hr;
#endif
      return hr;
    }
    callback->Wait();
    hr = listener_->EndClose(ctx.get());
    if (hr != S_OK) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(error) << "cannot EndClose: " << hr;
#endif
      return hr;
    }
    this->listener_ = nullptr;
    this->is_listening_ = false;
    return S_OK;
  }

private:
  winrt::com_ptr<IFabricTransportMessageHandler> req_handler_;
  winrt::com_ptr<IFabricTransportConnectionHandler> conn_handler_;
  winrt::com_ptr<IFabricTransportMessageDisposer> msg_disposer_;
  winrt::com_ptr<IFabricTransportListener> listener_;

  std::wstring hostname_;
  ULONG port_;
  std::wstring listening_addr_;
  bool is_listening_;
};

export class kv_client {
public:
  kv_client(winrt::com_ptr<IFabricTransportClient> client) : client_(client) {}

  HRESULT send(winrt::com_ptr<IFabricTransportMessage> const &request,
               winrt::com_ptr<IFabricTransportMessage> &reply) {
    HRESULT hr = S_OK;
    winrt::com_ptr<sf::IFabricAsyncOperationWaitableCallback> callback =
        winrt::make<sf::FabricAsyncOperationWaitableCallback>();
    winrt::com_ptr<IFabricAsyncOperationContext> ctx;
    hr = client_->BeginRequest(request.get(), 1000, callback.get(), ctx.put());

    if (hr != S_OK) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(error) << "cannot BeginRequest: " << hr;
#endif
      return hr;
    }
    callback->Wait();
    hr = client_->EndRequest(ctx.get(), reply.put());
    if (hr != S_OK) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(error) << "cannot EndRequest: " << hr;
#endif
      return hr;
    }
    return S_OK;
  }

private:
  winrt::com_ptr<IFabricTransportClient> client_;
};