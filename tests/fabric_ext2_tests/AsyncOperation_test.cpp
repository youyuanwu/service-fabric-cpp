#include "CompletedAsyncOperation.hpp"
#include "sf/AsyncOperation.hpp"
#include <boost/ut.hpp>
#include <latch>

boost::ut::suite async_operation_tests = [] {
  using namespace boost::ut;

  "basic_test"_test = [] {
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
      expect(!ec);
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
          expect(op->IsCompleted());
          expect(!ec);
        } else {
          ec = op->End();
          expect(ec == std::make_error_code(std::errc::operation_canceled));
        }
      } else {
        if (!op->IsCancelled()) {
          expect(op->IsCompleted());
          expect(!ec);
        } else {
          ec = op->End();
          expect(ec == std::make_error_code(std::errc::operation_canceled));
        }
      }
    }
  };
};

int main() {}