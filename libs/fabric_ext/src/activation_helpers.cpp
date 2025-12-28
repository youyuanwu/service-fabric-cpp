// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#include "servicefabric/activation_helpers.hpp"
// #include "FabricCommon.h"
#include "servicefabric/waitable_callback.hpp"
#include <spdlog/spdlog.h>
#include <winrt/base.h>

namespace servicefabric {

HRESULT get_hostname(std::wstring &hostname) {
  // get node ip
  winrt::com_ptr<IFabricAsyncOperationWaitableCallback> callback =
      winrt::make<FabricAsyncOperationWaitableCallback>();
  winrt::com_ptr<IFabricAsyncOperationContext> ctx;
  HRESULT hr = FabricBeginGetNodeContext(1000, callback.get(), ctx.put());
  if (hr != S_OK) {
    spdlog::error("FabricBeginGetNodeContext failed: {}", hr);
    return hr;
  }
  callback->Wait();
  winrt::com_ptr<IFabricNodeContextResult> node_ctx;
  hr = FabricEndGetNodeContext(ctx.get(), (void **)node_ctx.put());
  if (hr != S_OK) {
    spdlog::error("FabricEndGetNodeContext failed: {}", hr);
    return hr;
  }

  const FABRIC_NODE_CONTEXT *node_ctx_res = node_ctx->get_NodeContext();
  if (node_ctx_res == nullptr) {
    spdlog::error("FABRIC_NODE_CONTEXT is null");
    return hr;
  }
  spdlog::error(L"Node Ctx info/hostname: {}", node_ctx_res->IPAddressOrFQDN);

  hostname = node_ctx_res->IPAddressOrFQDN;
  return S_OK;
}

HRESULT
get_port(winrt::com_ptr<IFabricCodePackageActivationContext> activation_context,
         std::wstring const &endpoint_name, ULONG &port) {
  const FABRIC_ENDPOINT_RESOURCE_DESCRIPTION *echoEndpoint = nullptr;

  HRESULT hr = activation_context->GetServiceEndpointResource(
      endpoint_name.c_str(), &echoEndpoint);
  if (hr != S_OK) {
    spdlog::error("GetServiceEndpointResource failed: {}", hr);
    return hr;
  }
  spdlog::debug(
      L"Endpoint Info: name: {} port: {} protocol: {} type: {} cert: {}",
      echoEndpoint->Name, echoEndpoint->Port, echoEndpoint->Protocol,
      echoEndpoint->Type, echoEndpoint->CertificateName);
  port = echoEndpoint->Port;
  return S_OK;
}

} // namespace servicefabric