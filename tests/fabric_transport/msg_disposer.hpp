#pragma once

#include "fabrictransport_.h"
#include "servicefabric/async_context.hpp"
#include <boost/log/trivial.hpp>
#include <moderncom/interfaces.h>

class msg_disposer
    : public belt::com::object<msg_disposer, IFabricTransportMessageDisposer> {
public:
  void STDMETHODCALLTYPE Dispose(
      /* [in] */ ULONG Count,
      /* [size_is][in] */ IFabricTransportMessage **messages) override {
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug) << "msg_disposer::Dispose"
                             << " count " << Count;
#endif
  }
};