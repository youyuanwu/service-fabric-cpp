#define BOOST_TEST_MODULE fabric_asio_tests

#include <boost/test/unit_test.hpp>

#include "FabricClient.h"
#include "FabricTypes.h"

#include "servicefabric/fabric_error.hpp"
#include "servicefabric/winrt.hpp"

#pragma message("WinRT version " CPPWINRT_VERSION)

namespace sf = servicefabric;

BOOST_AUTO_TEST_SUITE(test_fabric_winrt)

// test winrt
BOOST_AUTO_TEST_CASE(test_winrt_waitable_callback) {
  winrt::init_apartment();

  belt::com::com_ptr<IFabricQueryClient> client;

  HRESULT hr =
      ::FabricCreateLocalClient(IID_IFabricQueryClient, (void **)client.put());

  BOOST_REQUIRE_EQUAL(hr, S_OK);

  auto f = [&]() -> winrt::Windows::Foundation::IAsyncAction {
    belt::com::com_ptr<sf::IWinRTAwaitableCallback> callback =
        sf::WinRTAwaitableCallback::create_instance().to_ptr();

    belt::com::com_ptr<IFabricAsyncOperationContext> ctx;
    FABRIC_NODE_QUERY_DESCRIPTION node = {};
    HRESULT hr =
        client->BeginGetNodeList(&node, 1000, callback.get(), ctx.put());

    BOOST_REQUIRE_EQUAL(hr, S_OK);
    // note that this call needs to be in a coroutine, and cannot be in main
    // function
    co_await callback->await();

    belt::com::com_ptr<IFabricGetNodeListResult> result;
    hr = client->EndGetNodeList(ctx.get(), result.put());

    if (hr != S_OK) {
      BOOST_TEST_MESSAGE("EndGetNodeList: " + sf::get_fabric_error_str(hr));
    }

    BOOST_REQUIRE_EQUAL(hr, S_OK);

    const FABRIC_NODE_QUERY_RESULT_LIST *nodes = result->get_NodeList();
    BOOST_CHECK_NE(nodes, nullptr);
  };

  auto action = f();
  action.get();

  winrt::uninit_apartment();
}

BOOST_AUTO_TEST_SUITE_END()