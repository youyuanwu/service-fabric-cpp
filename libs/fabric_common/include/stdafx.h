// dummy header. do not use pch
#pragma once

#define DENY_COPY(x)
#define DENY_COPY_ASSIGNMENT(x)

#define AcquireExclusiveLock std::lock_guard<std::mutex>
#define AcquireWriteLock std::lock_guard<std::mutex>
#define RwLock std::mutex
#define atomic_uint64 std::atomic<uint64_t>
#define CODING_ERROR_ASSERT assert
// #define atomic_bool std::atomic<bool>