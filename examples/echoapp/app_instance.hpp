// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#pragma once

#include "FabricCommon.h"
#include "FabricRuntime.h"
#include <winrt/base.h>

#include "echo_server.hpp"

#include <boost/asio.hpp>
#include <memory>
#include <thread>
#include <vector>

class app_instance
    : public winrt::implements<app_instance, IFabricStatelessServiceInstance> {
public:
  app_instance(ULONG port, std::wstring hostname);

  HRESULT STDMETHODCALLTYPE BeginOpen(
      /* [in] */ IFabricStatelessServicePartition *partition,
      /* [in] */ IFabricAsyncOperationCallback *callback,
      /* [retval][out] */ IFabricAsyncOperationContext **context) override;

  HRESULT STDMETHODCALLTYPE EndOpen(
      /* [in] */ IFabricAsyncOperationContext *context,
      /* [retval][out] */ IFabricStringResult **serviceAddress) override;

  HRESULT STDMETHODCALLTYPE BeginClose(
      /* [in] */ IFabricAsyncOperationCallback *callback,
      /* [retval][out] */ IFabricAsyncOperationContext **context) override;

  HRESULT STDMETHODCALLTYPE EndClose(
      /* [in] */ IFabricAsyncOperationContext *context) override;

  void STDMETHODCALLTYPE Abort(void) override;

private:
  std::unique_ptr<server> server_;
  boost::asio::io_context io_context_;
  ULONG port_;
  std::wstring hostname_;
  // background threads for server;
  std::vector<std::thread> server_threads_;
};