// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

#include "FabricClient.h"
#include "FabricTypes.h"
#include <boost/ut.hpp>

#include "servicefabric/fabric_error.hpp"
#include "servicefabric/winrt.hpp"

#pragma message("WinRT version " CPPWINRT_VERSION)

namespace sf = servicefabric;

boost::ut::suite errors = [] {
  using namespace boost::ut;

  // disabled.
  if (false) {
    "test_winrt_waitable_callback"_test = [] {
      winrt::init_apartment();

      winrt::com_ptr<IFabricQueryClient> client;

      HRESULT hr = ::FabricCreateLocalClient(IID_IFabricQueryClient,
                                             (void **)client.put());

      expect(hr == S_OK);

      auto f = [&]() -> winrt::Windows::Foundation::IAsyncAction {
        winrt::com_ptr<sf::IWinRTAwaitableCallback> callback =
            winrt::make<sf::WinRTAwaitableCallback>();

        winrt::com_ptr<IFabricAsyncOperationContext> ctx;
        FABRIC_NODE_QUERY_DESCRIPTION node = {};
        HRESULT hr =
            client->BeginGetNodeList(&node, 1000, callback.get(), ctx.put());

        expect(hr == S_OK);
        // note that this call needs to be in a coroutine, and cannot be in main
        // function
        co_await callback->await();

        winrt::com_ptr<IFabricGetNodeListResult> result;
        hr = client->EndGetNodeList(ctx.get(), result.put());

        if (hr != S_OK) {
          expect(false) << "EndGetNodeList: " + sf::get_fabric_error_str(hr);
        }

        expect(hr == S_OK);

        const FABRIC_NODE_QUERY_RESULT_LIST *nodes = result->get_NodeList();
        expect(nodes != nullptr);
      };

      auto action = f();
      action.get();

      winrt::uninit_apartment();
    };
  } // end if false

  // dummy test to avoid no tests error due to disable of tests
  "dummytest"_test = [] {};
};

int main() {}