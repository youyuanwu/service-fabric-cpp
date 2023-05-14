#include "ComponentB.hpp"
#include "CompletedAsyncOperation.hpp"

namespace sf {

AsyncOperationSPtr ComponentB::BeginFooB1(const AsyncCallback callback,
                                          AsyncOperationSPtr const &parent) {
  return sf::CreateAndStartAsyncOperation<CompletedAsyncOperation>(callback,
                                                                   parent);
}

std::error_code ComponentB::EndFooB1(AsyncOperationSPtr const &fooB1Operation) {
  return fooB1Operation->End();
}

} // namespace sf