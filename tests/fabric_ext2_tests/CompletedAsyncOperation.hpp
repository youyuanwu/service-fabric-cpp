#pragma once

#include "sf/AsyncOperation.hpp"
#include <thread>

class CompletedAsyncOperation : public sf::AsyncOperation {
public:
  CompletedAsyncOperation(sf::AsyncCallback const &callback,
                          std::shared_ptr<sf::AsyncOperation> const &parent);

protected:
  void OnStart() noexcept override;

private:
  // need to wait for th completion to avoid memleak.
  std::jthread th_;
};