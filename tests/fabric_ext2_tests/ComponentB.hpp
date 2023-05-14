#pragma once
#include <sf/AsyncOperation.hpp>

namespace sf {

class ComponentB {
public:
  ComponentB() {}

  sf::AsyncOperationSPtr BeginFooB1(const AsyncCallback callback,
                                    AsyncOperationSPtr const &parent);
  std::error_code EndFooB1(AsyncOperationSPtr const &fooB1Operation);
};

} // namespace sf