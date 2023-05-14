#include "sf/AsyncOperation.hpp"
#include <cassert>

namespace sf {

void AsyncOperation::OnCancel() noexcept {}

AsyncOperation::AsyncOperation() : AsyncOperation({}, {}) {}

AsyncOperation::AsyncOperation(AsyncCallback const &callback,
                               std::shared_ptr<AsyncOperation> const &parent)
    : parent_(parent), callback_(callback), child_(), mtx_(),
      state_(state::idle), ec_(), errMsg_(), completedSynchronously_(false) {
  // hook up parent with this in Start function.
}

void AsyncOperation::Start() noexcept {
  {
    std::lock_guard<std::mutex> lk(mtx_);
    assert(state_ == state::idle);
    if (parent_) {
      auto self = shared_from_this();
      parent_->AttachChild(self);
    }
    state_ = state::started;
  }
  // onstart may call other member functions
  // so we need to release lock early.
  OnStart();
  {
    std::lock_guard<std::mutex> lk(mtx_);
    completedSynchronously_ = state_ == state::completed;
  }
}

std::error_code AsyncOperation::End() noexcept {
  std::lock_guard<std::mutex> lk(mtx_);
  switch (state_) {
  case state::idle:
  case state::cancelling:
    // not in final state
    return std::make_error_code(std::errc::operation_not_supported);
  case state::started:
    return std::make_error_code(std::errc::operation_in_progress);
  case state::completed:
    return ec_;
  case state::cancelled:
    return std::make_error_code(std::errc::operation_canceled);
  default:
    assert(false);
    return std::make_error_code(std::errc::protocol_error);
  }
}

// this cancels all child operations
void AsyncOperation::Cancel() {
  {
    std::lock_guard<std::mutex> lk(mtx_);
    switch (state_) {
    case state::idle:
      assert(false); // operation not started.
      return;
    case state::cancelling:
    case state::cancelled:
      assert(false); // cancel already called.
      return;
    case state::started: {
      // One should only call cancel once.
      // if has child first cancel child, then cancel self.
      if (std::shared_ptr<AsyncOperation> c = child_.lock()) {
        c->Cancel();
        // detach
        child_.reset();
      }
      state_ = state::cancelling;
    } break;
    case state::completed: {
      // user has a race between cancel and complete.
      // since operation has completed we do nothing.
      std::shared_ptr<AsyncOperation> c = child_.lock();
      assert(!c);
      (c); // unreferenced var
    }
      return;
    default:
      assert(false);
      return;
    }
  }
  this->OnCancel();
  {
    std::lock_guard<std::mutex> lk(mtx_);
    switch (state_) {
    case state::cancelling:
      state_ = state::cancelled;
      break;
    case state::completed:
      // it cold happen that complete raced an win.
      break;
    default:
      // all other states are not valid.
      assert(false);
    }
  }
  // detach from parent
  DetachFromParent();
}

// this detaches self from parent in parent since self is completed.
void AsyncOperation::Complete(std::error_code ec, std::string errMsg) {
  std::unique_lock<std::mutex> lk(mtx_);
  switch (state_) {
  case state::started: {
    ec_ = ec;
    errMsg_ = errMsg;
    // child must be detached and finished
    std::shared_ptr<AsyncOperation> c = child_.lock();
    assert(!c);
    DetachFromParent();
    state_ = state::completed;
    // invoke user callback;
    assert(callback_);
    auto self = shared_from_this();
    // callback can reenter this object so we need to release lock
    lk.unlock();
    callback_(self);
    callback_ = nullptr;
  } break;
  case state::cancelled:
  case state::cancelling:
    // operation already cancelled. ignore.
    if (!ec_) {
      ec_ = std::make_error_code(std::errc::operation_canceled);
      errMsg_ = "Cannot complete when operation already canceled";
    }
    break;
  default:
    assert(false);
  }
}

void AsyncOperation::DetachFromParent() {
  if (parent_) {
    parent_->ResetChild();
    parent_ = {};
    // TODO: check parent has the same child.
  }
}

bool AsyncOperation::IsCancelled() {
  std::lock_guard<std::mutex> lk(mtx_);
  return state_ == state::cancelled || state_ == state::cancelling;
}

bool AsyncOperation::IsCompleted() {
  std::lock_guard<std::mutex> lk(mtx_);
  return state_ == state::completed;
}

bool AsyncOperation::IsCompletedSynchronously() {
  std::lock_guard<std::mutex> lk(mtx_);
  return completedSynchronously_;
}

// operation can only have one child at a time.
// so last child must be released.
void AsyncOperation::AttachChild(std::shared_ptr<AsyncOperation> child) {
  assert(child);
  auto c = child_.lock();
  assert(!c);
  (c);
  child_ = child;
}

void AsyncOperation::ResetChild() { child_ = {}; }

AsyncOperationRoot::AsyncOperationRoot() : AsyncOperation() {}

void AsyncOperationRoot::OnStart() noexcept {}

} // namespace sf