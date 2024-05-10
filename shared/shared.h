#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t
#include <iostream>

class BlockBase {
public:
    BlockBase() {
    }
    virtual void IncCounter() {
        ++count_;
    }
    virtual size_t GetCount() {
        return count_;
    }

    virtual void DecCounter() {
        --count_;
    }

    virtual ~BlockBase(){};

private:
    size_t count_ = 1;
};

template <class T>
class AllocatedByUser : public BlockBase {
public:
    AllocatedByUser() {
        object_ = nullptr;
    }

    AllocatedByUser(T* ptr) {
        object_ = ptr;
    }

    size_t GetCount() override {
        return count_;
    }

    void IncCounter() override {
        ++count_;
    }

    void DecCounter() override {
        --count_;
        if (count_ == 0) {
            delete object_;
            delete this;
        }
    }

    ~AllocatedByUser() override{};

private:
    T* object_;
    size_t count_ = 1;
};

template <class T>
class AllocatedByOurselves : public BlockBase {
public:
    template <typename... Args>
    AllocatedByOurselves(Args&&... args) {
        ::new (&object_) T(std::forward<Args>(args)...);
    };

    size_t GetCount() override {
        return count_;
    }

    T* GetObject() {
        return std::launder(reinterpret_cast<T*>(&object_));
    }

    void IncCounter() override {
        ++count_;
    }

    void DecCounter() override {
        --count_;
        if (count_ == 0) {
            std::destroy_at(std::launder(reinterpret_cast<T*>(&object_)));
            delete this;
        }
    }
    ~AllocatedByOurselves() override {
        std::destroy_at(std::launder(reinterpret_cast<T*>(&object_)));
    };

private:
    std::aligned_storage_t<sizeof(T), alignof(T)> object_;
    // T object_;
    size_t count_ = 1;
};

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T>
class SharedPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr() noexcept {
        object_ = nullptr;
        block_ = nullptr;
    };
    SharedPtr(std::nullptr_t) noexcept {
        object_ = nullptr;
        block_ = nullptr;
    };
    explicit SharedPtr(T* ptr) {
        object_ = ptr;
        AllocatedByUser<T>* block = new AllocatedByUser<T>(ptr);
        block_ = block;
    };

    template <class Y>
    explicit SharedPtr(Y* ptr) {
        object_ = ptr;
        AllocatedByUser<Y>* block = new AllocatedByUser<Y>(ptr);
        block_ = block;
    };

    SharedPtr(const SharedPtr& other) {
        object_ = other.Get();
        block_ = other.block_;
        if (Get() != nullptr) {
            block_->IncCounter();
        }
    };

    template <class Y>
    SharedPtr(const SharedPtr<Y>& other) {
        object_ = other.Get();
        block_ = other.GetBlock();
        if (Get() != nullptr) {
            block_->IncCounter();
        }
    };

    template <class Y>
    SharedPtr(SharedPtr<Y>&& other) {
        object_ = other.Get();
        block_ = other.GetBlock();
        block_->IncCounter();
        other.Reset();
    };

    SharedPtr(SharedPtr&& other) {
        object_ = other.Get();
        block_ = other.GetBlock();
        block_->IncCounter();
    };

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) {
        object_ = ptr;
        block_ = other.GetBlock();
        block_->IncCounter();
    };

    template <typename... Args>
    SharedPtr(bool f, Args&&... args) {
        AllocatedByOurselves<T>* block = new AllocatedByOurselves<T>(std::forward<Args>(args)...);
        object_ = nullptr;
        block_ = block;
    };

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other) {
        if (Get() == nullptr) {
            object_ = other.object_;
            block_ = other.block_;
        } else {
            object_ = other.object_;
            block_->DecCounter();
            block_ = other.block_;
        }
        if (Get() != nullptr) {
            block_->IncCounter();
        }
        return *this;
    };

    template <class Y>
    SharedPtr& operator=(const SharedPtr<Y>& other) {
        if (Get() == nullptr) {
            object_ = other.Get();
            block_ = other.GetBlock();
        } else {
            object_ = other.Get();
            block_->DecCounter();
            block_ = other.GetBlock();
        }
        if (Get() != nullptr) {
            block_->IncCounter();
        }
        return *this;
    };

    SharedPtr& operator=(SharedPtr&& other) {
        if (Get() != nullptr) {
            block_->DecCounter();
        }
        object_ = other.Get();
        block_ = other.GetBlock();
        if (object_ != nullptr) {
            block_->IncCounter();
        }
        other.Reset();
        return *this;
    };

    template <class Y>
    SharedPtr& operator=(SharedPtr<Y>&& other) {
        if (Get() != nullptr) {
            block_->DecCounter();
        }
        object_ = other.Get();
        block_ = other.GetBlock();
        if (object_ != nullptr) {
            block_->IncCounter();
        }
        other.Reset();
        return *this;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() {
        if (object_ != nullptr || block_ != nullptr) {
            block_->DecCounter();
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() noexcept {
        if (object_ != nullptr) {
            object_ = nullptr;
            block_->DecCounter();
            block_ = nullptr;
        }
    };

    template <class Y>
    void Reset(Y* ptr) {
        if (object_ != nullptr) {
            block_->DecCounter();
        }
        object_ = ptr;
        AllocatedByUser<Y>* block = new AllocatedByUser<Y>(ptr);
        block_ = block;
    };

    void Swap(SharedPtr& other) {
        std::swap(object_, other.object_);
        std::swap(block_, other.block_);
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        if (block_ != nullptr && object_ == nullptr) {
            AllocatedByOurselves<T>* block = dynamic_cast<AllocatedByOurselves<T>*>(block_);
            return block->GetObject();
        } else {
            return object_;
        }
    };

    BlockBase* GetBlock() const {
        return block_;
    }

    T& operator*() const {
        if (block_ != nullptr && object_ == nullptr) {
            AllocatedByOurselves<T>* block = dynamic_cast<AllocatedByOurselves<T>*>(block_);
            return *(block->GetObject());
        } else {
            return *object_;
        }
    };

    T* operator->() const {
        if (block_ != nullptr && object_ == nullptr) {
            AllocatedByOurselves<T>* block = dynamic_cast<AllocatedByOurselves<T>*>(block_);
            return block->GetObject();
        } else {
            return object_;
        }
    };

    size_t UseCount() const {
        if (object_ == nullptr) {
            return 0;
        } else {
            return block_->GetCount();
        }
    };

    explicit operator bool() const {
        return Get() != nullptr;
    };

private:
    T* object_;
    BlockBase* block_;
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    return left.Get() == right.Get();
};

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    SharedPtr<T> ptr(true, std::forward<Args>(args)...);
    return ptr;
};

// Look for usage examples in tests
template <typename T>
class EnableSharedFromThis {
public:
    SharedPtr<T> SharedFromThis();
    SharedPtr<const T> SharedFromThis() const;

    WeakPtr<T> WeakFromThis() noexcept;
    WeakPtr<const T> WeakFromThis() const noexcept;
};


