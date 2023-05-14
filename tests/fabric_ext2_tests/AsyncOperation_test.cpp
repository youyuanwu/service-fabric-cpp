#include "CompletedAsyncOperation.hpp"
#include "sf/AsyncOperation.hpp"
#include <boost/test/unit_test.hpp>
#include <latch>

BOOST_AUTO_TEST_SUITE(test_fabric_common_bare)

BOOST_AUTO_TEST_CASE(basic_test) {
  auto rootOp = sf::CreateAndStartAsyncOperation<sf::AsyncOperationRoot>();
  {
    std::error_code ec = {};
    std::latch sync(1);
    auto op = sf::CreateAndStartAsyncOperation<CompletedAsyncOperation>(
        [&ec, &sync](std::shared_ptr<sf::AsyncOperation> const &self) {
          ec = self->End();
          sync.count_down();
        },
        rootOp);

    sync.wait();
    BOOST_CHECK(!ec);
  }
  // cancel
  {
    std::error_code ec = {};
    std::latch sync(1);
    auto op = sf::CreateAndStartAsyncOperation<CompletedAsyncOperation>(
        [&ec, &sync](std::shared_ptr<sf::AsyncOperation> const &self) {
          ec = self->End();
          sync.count_down();
        },
        rootOp);
    op->Cancel();

    if (!op->IsCompletedSynchronously()) {
      if (!op->IsCancelled()) {
        sync.wait();
        BOOST_REQUIRE(op->IsCompleted());
        BOOST_REQUIRE(!ec);
      } else {
        ec = op->End();
        BOOST_REQUIRE_EQUAL(
            ec, std::make_error_code(std::errc::operation_canceled));
      }
    } else {
      if (!op->IsCancelled()) {
        BOOST_REQUIRE(op->IsCompleted());
        BOOST_REQUIRE(!ec);
      } else {
        ec = op->End();
        BOOST_REQUIRE_EQUAL(
            ec, std::make_error_code(std::errc::operation_canceled));
      }
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()