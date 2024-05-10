#pragma once

#include "compressed_pair.h"

#include <cstddef>  // std::nullptr_t

template <class T>
struct Slug {
    Slug() = default;

    template <class TT>
    Slug(Slug<TT>&& other) {
    }

    void operator()(T* p) {
        if (p != nullptr) {
            delete p;
        }
    }
};

template <class T>
struct Slug<T[]> {
    Slug() = default;

    template <class TT>
    Slug(Slug<TT>&& other) {
    }

    void operator()(T* p) {
        if (p != nullptr) {
            delete[] p;
        }
    }
};

// Primary template
template <typename T, typename Deleter = Slug<T>>
class UniquePtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) {
        my_ptr_.GetFirst() = std::move(ptr);
    };

    UniquePtr(T* ptr, Deleter deleter) noexcept {
        my_ptr_.GetFirst() = ptr;
        my_ptr_.GetSecond() = std::move(deleter);
    };

    template <class T1, class D1>
    UniquePtr(UniquePtr<T1, D1>&& other) noexcept {
        my_ptr_.GetFirst() = other.Release();
        my_ptr_.GetSecond() = std::move(other.my_ptr_.GetSecond());
    };

    UniquePtr(const UniquePtr& other) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s
    template <class T1, class D1>
    UniquePtr& operator=(UniquePtr<T1, D1>&& other) noexcept {
        if (my_ptr_.GetFirst() != other.my_ptr_.GetFirst()) {
            Reset(other.Release());
            my_ptr_.GetSecond() = std::move(other.my_ptr_.GetSecond());
        }
        return *this;
    };

    UniquePtr& operator=(const UniquePtr& other) = delete;

    UniquePtr& operator=(std::nullptr_t) {
        Reset(nullptr);
        return *this;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        my_ptr_.GetSecond()(my_ptr_.GetFirst());
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() noexcept {
        T* tmp_ptr = my_ptr_.GetFirst();
        my_ptr_.GetFirst() = nullptr;
        return tmp_ptr;
    };

    void Reset(T* ptr = nullptr) noexcept {
        T* tmp_ptr = my_ptr_.GetFirst();
        my_ptr_.GetFirst() = ptr;
        if (tmp_ptr != nullptr) {
            delete tmp_ptr;
        }
    };

    void Swap(UniquePtr& other) {
        std::swap(my_ptr_.GetSecond(), other.my_ptr_.GetSecond());
        std::swap(my_ptr_.GetFirst(), other.my_ptr_.GetFirst());
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return my_ptr_.GetFirst();
    };
    Deleter& GetDeleter() {
        return my_ptr_.GetSecond();
    };
    const Deleter& GetDeleter() const {
        return my_ptr_.GetSecond();
    };
    explicit operator bool() const {
        if (my_ptr_.GetFirst() == nullptr) {
            return false;
        } else {
            return true;
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    typename std::add_lvalue_reference<T>::type operator*() const noexcept {
        return *my_ptr_.GetFirst();
    };
    T* operator->() const noexcept {
        return my_ptr_.GetFirst();
    };

    CompressedPair<T*, Deleter> my_ptr_;
};

// Specialization for arrays
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) {
        my_ptr_.GetFirst() = std::move(ptr);
    };

    UniquePtr(T* ptr, Deleter deleter) noexcept {
        my_ptr_.GetFirst() = ptr;
        my_ptr_.GetSecond() = std::move(deleter);
    };

    template <class T1, class D1>
    UniquePtr(UniquePtr<T1, D1>&& other) noexcept {
        my_ptr_.GetFirst() = other.Release();
        my_ptr_.GetSecond() = std::move(other.my_ptr_.GetSecond());
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (my_ptr_.GetFirst() != other.Get()) {
            Reset(other.Release());
            my_ptr_.GetSecond() = std::move(other.my_ptr_.GetSecond());
        }
        return *this;
    };

    UniquePtr& operator=(std::nullptr_t) {
        Reset(nullptr);
        return *this;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        my_ptr_.GetSecond()(my_ptr_.GetFirst());
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() noexcept {
        T* tmp_ptr = my_ptr_.GetFirst();
        my_ptr_.GetFirst() = nullptr;
        return tmp_ptr;
    };

    void Reset(T* ptr = nullptr) noexcept {
        T* tmp_ptr = my_ptr_.GetFirst();
        my_ptr_.GetFirst() = ptr;
        if (tmp_ptr != nullptr) {
            delete[] tmp_ptr;
        }
    };

    void Swap(UniquePtr& other) {
        std::swap(my_ptr_.GetSecond(), other.my_ptr_.GetSecond());
        std::swap(my_ptr_.GetFirst(), other.my_ptr_.GetFirst());
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return my_ptr_.GetFirst();
    };
    Deleter& GetDeleter() {
        return my_ptr_.GetSecond();
    };
    const Deleter& GetDeleter() const {
        return my_ptr_.GetSecond();
    };
    explicit operator bool() const {
        if (my_ptr_.GetFirst() == nullptr) {
            return false;
        } else {
            return true;
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators
    T& operator[](std::size_t i) const {
        return my_ptr_.GetFirst()[i];
    };
    T& operator*() const noexcept {
        return *my_ptr_.GetFirst();
    };
    T* operator->() const noexcept {
        return my_ptr_.GetFirst();
    };

private:
    CompressedPair<T*, Deleter> my_ptr_;
};

