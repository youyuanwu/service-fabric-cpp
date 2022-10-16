#pragma once

#include "fabricservicecommunication_.h"
#include "servicefabric/async_context.hpp"
#include <moderncom/interfaces.h>

#include <string>

// namespace sf = servicefabric;

class message
    : public belt::com::object<message, IFabricServiceCommunicationMessage> {
public:
  message(std::string body, std::string headers)
      : body_(body), headers_(headers), body_ret_(), headers_ret_() {}

  FABRIC_MESSAGE_BUFFER *STDMETHODCALLTYPE Get_Body(void) override {
    body_ret_.BufferSize = static_cast<ULONG>(body_.size());
    body_ret_.Buffer = (BYTE *)body_.c_str();
    return &body_ret_;
  }

  FABRIC_MESSAGE_BUFFER *STDMETHODCALLTYPE Get_Headers(void) override {
    headers_ret_.BufferSize = static_cast<ULONG>(headers_.size());
    headers_ret_.Buffer = (BYTE *)headers_.c_str();
    return &headers_ret_;
  }

private:
  std::string body_;
  FABRIC_MESSAGE_BUFFER body_ret_;
  std::string headers_;
  FABRIC_MESSAGE_BUFFER headers_ret_;
};