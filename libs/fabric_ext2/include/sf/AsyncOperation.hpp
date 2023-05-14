#pragma once

#include <functional>
#include <memory>
#include <mutex>

namespace sf {

class AsyncOperation;

// argument is always operation self. arg is needed to bypass lambda capture
// where sometimes not possible
typedef std::function<void(std::shared_ptr<AsyncOperation> const &)>
    AsyncCallback;
typedef std::shared_ptr<AsyncOperation> AsyncOperationSPtr;

// might need to impl 0 arg
template <typename T, class... Args>
std::shared_ptr<AsyncOperation>
CreateAndStartAsyncOperation(Args &&...args) noexcept {
  static_assert(std::is_base_of<AsyncOperation, T>::value,
                "T must be derived from AsyncOperation");
  auto op = std::make_shared<T>(std::forward<Args>(args)...);
  op->Start();
  return op;
}

class AsyncOperation : public std::enable_shared_from_this<AsyncOperation> {
public:
  // if the async operation has no parent
  // bool IsRoot();

  // user needs to impl these
  virtual void OnStart() noexcept = 0;
  virtual void OnCancel() noexcept;

  // prepare and start the operation
  // it will call onstart.
  void Start() noexcept;

  // should be called after the operation has completed
  // returns the final error if any.
  std::error_code End() noexcept;

  // operation finally must be (either) canceled or completed.
  // User needs to call Cancel and Complete on different threads,
  // otherwise there is deadlock; Ideally only one should ever be called.
  void Cancel();
  void Complete(std::error_code ec = {}, std::string errMsg = {});

  bool IsCompleted();
  bool IsCancelled();
  bool IsCompletedSynchronously();

protected:
  // root
  AsyncOperation();
  // create a child that is attached to parent.
  AsyncOperation(AsyncCallback const &callback,
                 std::shared_ptr<AsyncOperation> const &parent);

private:
  void AttachChild(std::shared_ptr<AsyncOperation> child);
  void ResetChild();

  // clean up the link to parent since operation completed or cancelled.
  void DetachFromParent();

  std::shared_ptr<AsyncOperation> parent_;
  AsyncCallback callback_;

  // may change to child list?
  std::weak_ptr<AsyncOperation> child_;

  // protects state
  std::mutex mtx_;
  enum state { idle, started, cancelling, cancelled, completed } state_;

  std::error_code ec_;
  std::string errMsg_;
  bool completedSynchronously_;
};

// operation that does not have parent or callback
class AsyncOperationRoot : public AsyncOperation {
public:
  AsyncOperationRoot();
  void OnStart() noexcept override;
};

} // namespace sf