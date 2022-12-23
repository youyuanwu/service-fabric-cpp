#include "servicefabric/transport_dummy_msg_disposer.hpp"

namespace servicefabric {

void STDMETHODCALLTYPE transport_dummy_msg_disposer::Dispose(
    /* [in] */ ULONG Count,
    /* [size_is][in] */ IFabricTransportMessage **messages) {
  UNREFERENCED_PARAMETER(Count);
  UNREFERENCED_PARAMETER(messages);
#ifdef SF_DEBUG
  BOOST_LOG_TRIVIAL(debug) << "msg_disposer::Dispose"
                           << " count " << Count;
#endif
}

} // namespace servicefabric