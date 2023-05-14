#include "CompletedAsyncOperation.hpp"
#include <random>

CompletedAsyncOperation::CompletedAsyncOperation(
    sf::AsyncCallback const &callback,
    std::shared_ptr<sf::AsyncOperation> const &parent)
    : AsyncOperation(callback, parent), th_() {}

void CompletedAsyncOperation::OnStart() noexcept {
  // flip randomly to use sync or async.
  static thread_local std::mt19937 generator(clock());
  std::uniform_int_distribution<int> distribution(0, 2000);
  auto delayMs = distribution(generator);
  bool async = delayMs % 2 == 1;
  // async = true;
  if (async) {
    // auto self = shared_from_this();
    //  this owns th_ so we guarantee that this is valid until thread finish
    th_ = std::jthread([this, delayMs]() {
      std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
      this->Complete();
    });
  } else {
    this->Complete();
  }
}