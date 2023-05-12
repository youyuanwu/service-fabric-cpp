// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#pragma once

#include "fabricservicecommunication_.h"
#include "servicefabric/async_context.hpp"
#include <moderncom/interfaces.h>

namespace sf = servicefabric;

class conn_handler
    : public belt::com::object<conn_handler, IFabricServiceConnectionHandler> {
public:
  HRESULT STDMETHODCALLTYPE BeginProcessConnect(
      /* [in] */ IFabricClientConnection *clientConnection,
      /* [in] */ DWORD timeoutMilliseconds,
      /* [in] */ IFabricAsyncOperationCallback *callback,
      /* [retval][out] */ IFabricAsyncOperationContext **context) override {
    UNREFERENCED_PARAMETER(clientConnection);
    UNREFERENCED_PARAMETER(timeoutMilliseconds);
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug) << "conn_handler::BeginProcessConnect";
#endif

    belt::com::com_ptr<IFabricAsyncOperationContext> ctx =
        sf::async_context::create_instance(callback).to_ptr();

    *context = ctx.detach();

    return S_OK;
  };

  HRESULT STDMETHODCALLTYPE EndProcessConnect(
      /* [in] */ IFabricAsyncOperationContext *context) override {
    UNREFERENCED_PARAMETER(context);
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug) << "conn_handler::EndProcessConnect";
#endif
    return S_OK;
  };

  HRESULT STDMETHODCALLTYPE BeginProcessDisconnect(
      /* [in] */ LPCWSTR clientId,
      /* [in] */ DWORD timeoutMilliseconds,
      /* [in] */ IFabricAsyncOperationCallback *callback,
      /* [retval][out] */ IFabricAsyncOperationContext **context) override {
    UNREFERENCED_PARAMETER(clientId);
    UNREFERENCED_PARAMETER(timeoutMilliseconds);
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug) << "conn_handler::BeginProcessDisconnect";
#endif
    belt::com::com_ptr<IFabricAsyncOperationContext> ctx =
        sf::async_context::create_instance(callback).to_ptr();
    *context = ctx.detach();
    return S_OK;
  };

  HRESULT STDMETHODCALLTYPE EndProcessDisconnect(
      /* [in] */ IFabricAsyncOperationContext *context) override {
    UNREFERENCED_PARAMETER(context);
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug) << "conn_handler::EndProcessDisconnect";
#endif
    return S_OK;
  };
};