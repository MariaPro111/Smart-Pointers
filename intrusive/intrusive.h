#pragma once

#include <cstddef>  // for std::nullptr_t
#include <utility>  // for std::exchange / std::swap

class SimpleCounter {
public:
    size_t IncRef() {
        ++count_;
        return count_;
    };
    size_t DecRef() {
        --count_;
        return count_;
    };
    size_t RefCount() const {
        return count_;
    };

private:
    size_t count_ = 0;
};

struct DefaultDelete {
    template <typename T>
    static void Destroy(T* object) {
        delete object;
    }
};

template <typename Derived, typename Counter, typename Deleter>
class RefCounted {
public:
    // Increase reference counter.
    void IncRef() {
        counter_.IncRef();
    };

    // Decrease reference counter.
    // Destroy object using Deleter when the last instance dies.
    void DecRef() {
        counter_.DecRef();
        if (counter_.RefCount() == 0) {
            Deleter::Destroy(static_cast<Derived*>(this));
        };
    };

    RefCounted& operator=(const RefCounted& other) {
        return *this;
    };

    // Get current counter value (the number of strong references).
    size_t RefCount() const {
        return counter_.RefCount();
    };

private:
    Counter counter_;
};

template <typename Derived, typename D = DefaultDelete>
using SimpleRefCounted = RefCounted<Derived, SimpleCounter, D>;

template <typename T>
class IntrusivePtr {
    template <typename Y>
    friend class IntrusivePtr;

public:
    // Constructors
    IntrusivePtr() {
        ptr_ = nullptr;
    };
    IntrusivePtr(std::nullptr_t) {
        ptr_ = nullptr;
    };
    IntrusivePtr(T* ptr) : ptr_(ptr) {
        ptr_->IncRef();
    };

    template <class Y>
    IntrusivePtr(Y* ptr) : ptr_(ptr) {
        ptr_->IncRef();
    };

    template <typename Y>
    IntrusivePtr(const IntrusivePtr<Y>& other) {
        ptr_ = other.Get();
        if (ptr_ != nullptr) {
            ptr_->IncRef();
        }
    };

    template <typename Y>
    IntrusivePtr(IntrusivePtr<Y>&& other) {
        ptr_ = other.Get();
        if (ptr_ != nullptr) {
            ptr_->IncRef();
        }
        other.Reset();
    };

    IntrusivePtr(const IntrusivePtr& other) {
        ptr_ = other.Get();
        if (ptr_ != nullptr) {
            ptr_->IncRef();
        }
    };
    IntrusivePtr(IntrusivePtr&& other) {
        ptr_ = other.Get();
        if (ptr_ != nullptr) {
            ptr_->IncRef();
        }
        other.Reset();
    };

    template <typename... Args>
    IntrusivePtr(bool f, Args&&... args) {
        T* p = new T(std::forward<Args>(args)...);
        ptr_ = p;
        if (ptr_ != nullptr) {
            ptr_->IncRef();
        }
    };

    // `operator=`-s
    IntrusivePtr& operator=(const IntrusivePtr& other) {
        if (ptr_ == other.Get()) {
            return *this;
        }
        if (ptr_ != nullptr) {
            ptr_->DecRef();
            ptr_ = other.Get();
            if (ptr_ != nullptr) {
                ptr_->IncRef();
            }
        }
        return *this;
    };
    IntrusivePtr& operator=(IntrusivePtr&& other) {
        if (ptr_ == other.Get()) {
            return *this;
        }
        if (ptr_ != nullptr) {
            ptr_->DecRef();
        }
        ptr_ = other.Get();
        if (ptr_ != nullptr) {
            ptr_->IncRef();
        }
        other.Reset();
        return *this;
    };

    // Destructor
    ~IntrusivePtr() {
        if (ptr_ != nullptr) {
            ptr_->DecRef();
        }
    };

    // Modifiers
    void Reset() {
        if (ptr_ != nullptr) {
            ptr_->DecRef();
        }
        ptr_ = nullptr;
    };
    void Reset(T* ptr) {
        if (ptr_ != nullptr) {
            ptr_->DecRef();
        }
        ptr_ = ptr;
        if (ptr_ != nullptr) {
            ptr_->IncRef();
        }
    };
    void Swap(IntrusivePtr& other) {
        if (ptr_ != other.Get()) {
            std::swap(ptr_, other.ptr_);
        }
    };

    // Observers
    T* Get() const {
        return ptr_;
    };
    T& operator*() const {
        return *ptr_;
    };
    T* operator->() const {
        return ptr_;
    };
    size_t UseCount() const {
        if (ptr_ == nullptr) {
            return 0;
        }
        return ptr_->RefCount();
    };
    explicit operator bool() const {
        if (ptr_ == nullptr) {
            return false;
        }
        if (ptr_->RefCount() > 0) {
            return true;
        } else {
            return false;
        }
    };

private:
    T* ptr_;
    // RefCounted ref_count_;
};

template <typename T, typename... Args>
IntrusivePtr<T> MakeIntrusive(Args&&... args) {
    IntrusivePtr<T> ptr(true, std::forward<Args>(args)...);
    return ptr;
};

