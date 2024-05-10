#pragma once

#include "sw_fwd.h"  // Forward declaration
#include "shared.h"
// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr() {
        object_ = nullptr;
        block_ = nullptr;
    };

    WeakPtr(const WeakPtr& other) {
        object_ = other.Get();
        block_ = other.GetBlock();
        if (Get() != nullptr) {
            block_->IncCounterWeak();
        }
    };
    WeakPtr(WeakPtr&& other) {
        object_ = other.Get();
        block_ = other.GetBlock();
        block_->IncCounterWeak();
        other.Reset();
    };

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    WeakPtr(const SharedPtr<T>& other) {
        object_ = other.Get();
        block_ = other.GetBlock();
        if (GetBlock() != nullptr) {
            block_->IncCounterWeak();
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) {
        if (Get() == nullptr) {
            object_ = other.Get();
            block_ = other.GetBlock();
        } else {
            object_ = other.Get();
            block_->DecCounterWeak();
            block_ = other.block_;
        }
        if (Get() != nullptr) {
            block_->IncCounterWeak();
        }
        return *this;
    };

    WeakPtr& operator=(const SharedPtr<T>& other) {
        if (Get() == nullptr) {
            object_ = other.Get();
            block_ = other.GetBlock();
        } else {
            object_ = other.Get();
            block_->DecCounterWeak();
            block_ = other.GetBlock();
        }
        if (Get() != nullptr) {
            block_->IncCounterWeak();
        }
        return *this;
    };
    WeakPtr& operator=(WeakPtr&& other) {
        if (Get() != nullptr) {
            block_->DecCounterWeak();
        }
        object_ = other.Get();
        block_ = other.GetBlock();
        if (object_ != nullptr) {
            block_->IncCounterWeak();
        }
        other.Reset();
        return *this;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        if (block_ != nullptr && block_->GetCountWeak() > 0) {
            block_->DecCounterWeak();
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        if (block_ != nullptr) {
            object_ = nullptr;
            block_->DecCounterWeak();
            block_ = nullptr;
        }
    };

    void Swap(WeakPtr& other) {
        std::swap(object_, other.object_);
        std::swap(block_, other.block_);
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        if (UseCount() == 0) {
            return nullptr;
        }
        if (block_ != nullptr && object_ == nullptr) {
            AllocatedByOurselves<T>* block = dynamic_cast<AllocatedByOurselves<T>*>(block_);
            return block->GetObject();
        } else {
            return object_;
        }
    }
    BlockBase* GetBlock() const {
        return block_;
    }
    size_t UseCount() const {
        if (block_ == nullptr) {
            return 0;
        } else {
            return block_->GetCount();
        }
    };
    bool Expired() const noexcept {
        if (block_ != nullptr) {
            if (block_->GetCount() == 0) {
                return true;
            } else {
                return false;
            };
        } else {
            return true;
        }
    };
    SharedPtr<T> Lock() const {
        if (Get() == nullptr) {
            SharedPtr<T> ptr;
            return ptr;
        } else {
            /*if(block_->GetCountWeak()>0){
                block_->DecCounterWeak();
            }*/
            SharedPtr<T> ptr(*this);

            return ptr;
        }
    };

private:
    T* object_;
    BlockBase* block_;
};

