#ifndef INCLUDE_UTILITIES_LOCKS_HPP_
#define INCLUDE_UTILITIES_LOCKS_HPP_

#include <mutex>
#include <shared_mutex>

// Normal mutex lock
using Lock = std::unique_lock<std::mutex>;

// For a shared mutex, if you want to acquire in shared or exclusive mode
using ReadLock = std::shared_lock<std::shared_mutex>;
using WriteLock = std::unique_lock<std::shared_mutex>;

#endif  // INCLUDE_UTILITIES_LOCKS_HPP_
