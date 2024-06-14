#ifndef INCLUDE_UTILITIES_LOCKPTR_HPP_
#define INCLUDE_UTILITIES_LOCKPTR_HPP_

#include <memory>
#include <mutex>

template <typename T>
class LockPtr {
 public:
    std::shared_ptr<T> data;

    LockPtr(std::shared_ptr<T> ptr, std::mutex* mut): mut {mut}, data {ptr} {
        mut->lock();
    }

    ~LockPtr() {
        this->mut->unlock();
    }

 private:
    std::mutex* mut;
};

#endif  // INCLUDE_UTILITIES_LOCKPTR_HPP_
