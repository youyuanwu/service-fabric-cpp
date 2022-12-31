#pragma once

#include "FabricCommon.h"

#include <moderncom/interfaces.h>

#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/base.h>
namespace servicefabric {

MIDL_INTERFACE("1b8bdf9a-b527-4543-94c8-63ea0536e365")
IWinRTAwaitableCallback : public IFabricAsyncOperationCallback {
public:
  // for co_await
  virtual winrt::Windows::Foundation::IAsyncAction await() = 0;
  virtual winrt::handle &get_event() = 0;
};

class WinRTAwaitableCallback
    : public belt::com::object<WinRTAwaitableCallback,
                               IWinRTAwaitableCallback> {
public:
  WinRTAwaitableCallback()
      : event_(::CreateEvent(nullptr, // attribute
                             true,    // manual reset
                             false,   // initial state
                             nullptr  // name
                             )) {
    assert(event_.get() != nullptr);
  }

  void Invoke(/* [in] */ IFabricAsyncOperationContext *context) override {
    UNREFERENCED_PARAMETER(context);
    [[maybe_unused]] bool ok = ::SetEvent(event_.get());
    assert(ok);
  }

  winrt::Windows::Foundation::IAsyncAction await() override {
    co_await winrt::resume_on_signal(event_.get());
  }

  winrt::handle &get_event() override { return event_; }

private:
  winrt::handle event_; // this is auto closed on destruction.
};

} // namespace servicefabric