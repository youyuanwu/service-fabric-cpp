#include "servicefabric/transport_message.hpp"

namespace servicefabric {

transport_message::transport_message(std::string body, std::string headers)
    : body_(body), headers_(headers), body_ret_(), headers_ret_() {}

void STDMETHODCALLTYPE transport_message::GetHeaderAndBodyBuffer(
    /* [out] */ const FABRIC_TRANSPORT_MESSAGE_BUFFER **headerBuffer,
    /* [out] */ ULONG *msgBufferCount,
    /* [out] */ const FABRIC_TRANSPORT_MESSAGE_BUFFER **MsgBuffers) {
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

void STDMETHODCALLTYPE transport_message::Dispose(void) {
#ifdef SF_DEBUG
  BOOST_LOG_TRIVIAL(debug) << "message::Dispose";
#endif
}

std::string get_header(IFabricTransportMessage *message) {
  if (message == nullptr) {
    return "";
  }
  const FABRIC_TRANSPORT_MESSAGE_BUFFER *headerbuf = {};
  const FABRIC_TRANSPORT_MESSAGE_BUFFER *msgbuf = {};
  ULONG msgcount = 0;
  message->GetHeaderAndBodyBuffer(&headerbuf, &msgcount, &msgbuf);
  return std::string(headerbuf->Buffer,
                     headerbuf->Buffer + headerbuf->BufferSize);
}

// concat all body chunks to one
std::string get_body(IFabricTransportMessage *message) {
  if (message == nullptr) {
    return "";
  }
  const FABRIC_TRANSPORT_MESSAGE_BUFFER *headerbuf = {};
  const FABRIC_TRANSPORT_MESSAGE_BUFFER *msgbuf = {};
  ULONG msgcount = 0;
  message->GetHeaderAndBodyBuffer(&headerbuf, &msgcount, &msgbuf);

  std::string body;
  for (std::size_t i = 0; i < msgcount; i++) {
    const FABRIC_TRANSPORT_MESSAGE_BUFFER *msg_i = msgbuf + i;
    std::string msg_str(msg_i->Buffer, msg_i->Buffer + msg_i->BufferSize);
    body += msg_str;
  }
  return body;
}

} // namespace servicefabric