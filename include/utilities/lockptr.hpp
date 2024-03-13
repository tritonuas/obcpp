#ifndef INCLUDE_UTILITIES_LOCKREF_HPP_
#define INCLUDE_UTILITIES_LOCKREF_HPP_

#include <memory>
#include <mutex>

template <typename T>
class LockPtr {
 public:
    std::shared_ptr<T> ptr;

    LockPtr(std::shared_ptr<T> ptr, std::mutex& mut): mut {mut} {
        mut.lock();
        this->ptr = ptr;
    }

    ~LockPtr() {
        this->mut.unlock();
    }

 private:
    std::mutex& mut;
};

#endif  // INCLUDE_UTILITIES_LOCKREF_HPP_