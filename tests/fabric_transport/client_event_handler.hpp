#pragma once
#include <boost/log/trivial.hpp>

#include "fabrictransport_.h"
#include "servicefabric/async_context.hpp"
#include <moderncom/interfaces.h>

class client_event_handler
    : public belt::com::object<client_event_handler,
                               IFabricTransportClientEventHandler> {
public:
  HRESULT STDMETHODCALLTYPE OnConnected(
      /* [in] */ LPCWSTR connectionAddress) override {
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug)
        << "client_event_handler::OnConnected addr: " << connectionAddress;
#endif
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE OnDisconnected(
      /* [in] */ LPCWSTR connectionAddress,
      /* [in] */ HRESULT error) override {
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug)
        << "client_event_handler::OnDisconnected addr: " << connectionAddress;
#endif
    return S_OK;
  }
};