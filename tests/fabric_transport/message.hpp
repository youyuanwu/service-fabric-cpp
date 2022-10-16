#pragma once

#include "fabrictransport_.h"
#include "servicefabric/async_context.hpp"
#include <boost/log/trivial.hpp>
#include <moderncom/interfaces.h>

#include <string>

class message : public belt::com::object<message, IFabricTransportMessage> {
public:
  message(std::string body, std::string headers)
      : body_(body), headers_(headers), body_ret_(), headers_ret_() {}

  void STDMETHODCALLTYPE GetHeaderAndBodyBuffer(
      /* [out] */ const FABRIC_TRANSPORT_MESSAGE_BUFFER **headerBuffer,
      /* [out] */ ULONG *msgBufferCount,
      /* [out] */ const FABRIC_TRANSPORT_MESSAGE_BUFFER **MsgBuffers) override {
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug) << "message::GetHeaderAndBodyBuffer";
#endif
    // todo: return only parts that has the pointers.
    if (headerBuffer == nullptr || msgBufferCount == nullptr ||
        MsgBuffers == nullptr) {
      return;
    }
    // prepare return
    headers_ret_.Buffer = (BYTE *)headers_.c_str();
    headers_ret_.BufferSize = static_cast<ULONG>(headers_.size());
    body_ret_.Buffer = (BYTE *)body_.c_str();
    body_ret_.BufferSize = static_cast<ULONG>(body_.size());

    *headerBuffer = &headers_ret_;
    *msgBufferCount = 1;
    *MsgBuffers = &body_ret_;
  }

  void STDMETHODCALLTYPE Dispose(void) override {
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug) << "message::Dispose";
#endif
  }

private:
  std::string body_;
  FABRIC_TRANSPORT_MESSAGE_BUFFER body_ret_;
  std::string headers_;
  FABRIC_TRANSPORT_MESSAGE_BUFFER headers_ret_;
};
