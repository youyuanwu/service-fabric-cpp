// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#include "servicefabric/activation_helpers.hpp"
// #include "FabricCommon.h"
#include "servicefabric/waitable_callback.hpp"
#include <boost/log/trivial.hpp>
#include <moderncom/interfaces.h>

namespace servicefabric {

HRESULT get_hostname(std::wstring &hostname) {
  // get node ip
  belt::com::com_ptr<IFabricAsyncOperationWaitableCallback> callback =
      FabricAsyncOperationWaitableCallback::create_instance().to_ptr();
  belt::com::com_ptr<IFabricAsyncOperationContext> ctx;
  HRESULT hr = FabricBeginGetNodeContext(1000, callback.get(), ctx.put());
  if (hr != S_OK) {
    BOOST_LOG_TRIVIAL(error) << "FabricBeginGetNodeContext failed: " << hr;
    return hr;
  }
  callback->Wait();
  belt::com::com_ptr<IFabricNodeContextResult> node_ctx;
  hr = FabricEndGetNodeContext(ctx.get(), (void **)node_ctx.put());
  if (hr != S_OK) {
    BOOST_LOG_TRIVIAL(error) << "FabricEndGetNodeContext failed: " << hr;
    return hr;
  }

  const FABRIC_NODE_CONTEXT *node_ctx_res = node_ctx->get_NodeContext();
  if (node_ctx_res == nullptr) {
    BOOST_LOG_TRIVIAL(error) << "FABRIC_NODE_CONTEXT is null";
    return hr;
  }
  BOOST_LOG_TRIVIAL(error) << "Node Ctx info/hostname:"
                           << node_ctx_res->IPAddressOrFQDN;

  hostname = node_ctx_res->IPAddressOrFQDN;
  return S_OK;
}

HRESULT
get_port(belt::com::ref<IFabricCodePackageActivationContext> activation_context,
         std::wstring const &endpoint_name, ULONG &port) {
  const FABRIC_ENDPOINT_RESOURCE_DESCRIPTION *echoEndpoint = nullptr;

  HRESULT hr = activation_context->GetServiceEndpointResource(
      endpoint_name.c_str(), &echoEndpoint);
  if (hr != S_OK) {
    BOOST_LOG_TRIVIAL(error) << "GetServiceEndpointResource failed: " << hr;
    return hr;
  }
  BOOST_LOG_TRIVIAL(debug) << "Endpoint Info: "
                           << " name: " << echoEndpoint->Name
                           << " port: " << echoEndpoint->Port
                           << " protocol: " << echoEndpoint->Protocol
                           << " type: " << echoEndpoint->Type
                           << " cert: " << echoEndpoint->CertificateName;
  port = echoEndpoint->Port;
  return S_OK;
}

} // namespace servicefabric