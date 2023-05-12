// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#pragma once

#include "FabricCommon.h"
#include "fabrictransport_.h"
// #include "FabricRuntime.h"
#include <moderncom/interfaces.h>

namespace servicefabric {

class transport_message
    : public belt::com::object<transport_message, IFabricTransportMessage> {
public:
  transport_message(std::string body, std::string headers);

  void STDMETHODCALLTYPE GetHeaderAndBodyBuffer(
      /* [out] */ const FABRIC_TRANSPORT_MESSAGE_BUFFER **headerBuffer,
      /* [out] */ ULONG *msgBufferCount,
      /* [out] */ const FABRIC_TRANSPORT_MESSAGE_BUFFER **MsgBuffers) override;

  void STDMETHODCALLTYPE Dispose(void) override;

private:
  std::string body_;
  FABRIC_TRANSPORT_MESSAGE_BUFFER body_ret_;
  std::string headers_;
  FABRIC_TRANSPORT_MESSAGE_BUFFER headers_ret_;
};

std::string get_header(IFabricTransportMessage *message);
// concat all body chunks to one
std::string get_body(IFabricTransportMessage *message);

} // namespace servicefabric