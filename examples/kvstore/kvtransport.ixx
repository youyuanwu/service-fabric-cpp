module;
#include "fabrictransport_.h"
#include <moderncom/interfaces.h>

#include <boost/log/trivial.hpp>

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
    : public belt::com::object<request_handler,
                               IFabricTransportMessageHandler> {
public:
  HRESULT STDMETHODCALLTYPE BeginProcessRequest(
      /* [in] */ COMMUNICATION_CLIENT_ID clientId,
      /* [in] */ IFabricTransportMessage *message,
      /* [in] */ DWORD timeoutMilliseconds,
      /* [in] */ IFabricAsyncOperationCallback *callback,
      /* [retval][out] */ IFabricAsyncOperationContext **context) override {
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

    belt::com::com_ptr<IFabricAsyncOperationContext> ctx =
        sf::async_context::create_instance(callback).to_ptr();
    *context = ctx.detach();
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE EndProcessRequest(
      /* [in] */ IFabricAsyncOperationContext *context,
      /* [retval][out] */ IFabricTransportMessage **reply) override {

#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug) << "request_handler::EndProcessRequest";
#endif
    belt::com::com_ptr<IFabricTransportMessage> msg1 =
        sf::transport_message::create_instance("mybodyreply", "myheaderreply")
            .to_ptr();
    *reply = msg1.detach();
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE HandleOneWay(
      /* [in] */ COMMUNICATION_CLIENT_ID clientId,
      /* [in] */ IFabricTransportMessage *message) override {
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug) << "request_handler::HandleOneWay";
#endif
    return S_OK;
  }
};

export class kv_server {
public:
  kv_server(std::wstring hostname, ULONG port)
      : req_handler_(request_handler::create_instance().to_ptr()),
        conn_handler_(sf::transport_dummy_server_conn_handler::create_instance()
                          .to_ptr()),
        msg_disposer_(
            sf::transport_dummy_msg_disposer::create_instance().to_ptr()),
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
      BOOST_LOG_TRIVIAL(error)
          << "cannot CreateFabricTransportListener: " << hr;
      return hr;
    }

    // open listener
    belt::com::com_ptr<sf::IFabricAsyncOperationWaitableCallback> callback =
        sf::FabricAsyncOperationWaitableCallback::create_instance().to_ptr();
    belt::com::com_ptr<IFabricAsyncOperationContext> ctx;
    hr = listener_->BeginOpen(callback.get(), ctx.put());
    if (hr != S_OK) {
      BOOST_LOG_TRIVIAL(error) << "cannot BeginOpen: " << hr;
      return hr;
    }
    callback->Wait();
    belt::com::com_ptr<IFabricStringResult> addr_str;
    hr = listener_->EndOpen(ctx.get(), addr_str.put());
    if (hr != S_OK) {
      BOOST_LOG_TRIVIAL(error) << "cannot EndOpen: " << hr;
      return hr;
    }
    this->listening_addr_ = std::wstring(addr_str->get_String());
    BOOST_LOG_TRIVIAL(debug) << "Listening on address: " << listening_addr_;
    this->is_listening_ = true;
    return S_OK;
  }

  std::wstring get_listening_addr() { return this->listening_addr_; }

  // can call multiple times
  HRESULT close() {
    if (!this->is_listening_) {
      return S_OK;
    }
    belt::com::com_ptr<sf::IFabricAsyncOperationWaitableCallback> callback =
        sf::FabricAsyncOperationWaitableCallback::create_instance().to_ptr();
    belt::com::com_ptr<IFabricAsyncOperationContext> ctx;
    HRESULT hr;
    hr = listener_->BeginClose(callback.get(), ctx.put());

    if (hr != S_OK) {
      BOOST_LOG_TRIVIAL(error) << "cannot BeginClose: " << hr;
      return hr;
    }
    callback->Wait();
    hr = listener_->EndClose(ctx.get());
    if (hr != S_OK) {
      BOOST_LOG_TRIVIAL(error) << "cannot EndClose: " << hr;
      return hr;
    }
    this->listener_.reset();
    this->is_listening_ = false;
    return S_OK;
  }

private:
  belt::com::com_ptr<IFabricTransportMessageHandler> req_handler_;
  belt::com::com_ptr<IFabricTransportConnectionHandler> conn_handler_;
  belt::com::com_ptr<IFabricTransportMessageDisposer> msg_disposer_;
  belt::com::com_ptr<IFabricTransportListener> listener_;

  std::wstring hostname_;
  ULONG port_;
  std::wstring listening_addr_;
  bool is_listening_;
};