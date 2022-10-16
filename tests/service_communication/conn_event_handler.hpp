#pragma once

#include <boost/log/trivial.hpp>

#include "fabricservicecommunication_.h"
#include "servicefabric/async_context.hpp"
#include <moderncom/interfaces.h>

namespace sf = servicefabric;

// for client
class conn_event_handler
    : public belt::com::object<conn_event_handler,
                               IFabricServiceConnectionEventHandler> {
public:
  HRESULT STDMETHODCALLTYPE OnConnected(
      /* [in] */ LPCWSTR connectionAddress) override {
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug)
        << "conn_event_handler::OnConnected connectionAddress: "
        << connectionAddress;
#endif
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE OnDisconnected(
      /* [in] */ LPCWSTR connectionAddress,
      /* [in] */ HRESULT error) override {
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug)
        << "conn_event_handler::OnDisconnected connectionAddress: "
        << connectionAddress << " error: " << error;
#endif
    return S_OK;
  }
};