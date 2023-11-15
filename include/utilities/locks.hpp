#ifndef UTILITIES_LOCKS_HPP
#define UTILITIES_LOCKS_HPP

#include <mutex>
#include <shared_mutex>

// Normal mutex lock
using Lock = std::unique_lock<std::mutex>;

// For a shared mutex, if you want to acquire in shared or exclusive mode
using ReadLock = std::shared_lock<std::shared_mutex>;
using WriteLock = std::unique_lock<std::shared_mutex>;

#endif // UTILITIES_LOCKS_HPP