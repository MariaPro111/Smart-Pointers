#pragma once

// Paste here your implementation of compressed_pair from seminar 2 to use in UniquePtr

#include <type_traits>
#include <utility>

// Me think, why waste time write lot code, when few code do trick.

template <typename F, typename S, bool T, bool K, bool C>
class CompressedPairHelp;

template <typename F, typename S>
class CompressedPairHelp<F, S, true, true, true> : public F {
public:
    CompressedPairHelp(){};
    CompressedPairHelp(F&& first, S&& second) {
    }

private:
    F first_;
};

template <typename F, typename S>
class CompressedPairHelp<F, S, true, true, false> : public F, public S {
public:
    CompressedPairHelp(){};
    template <typename FF, typename SS>
    CompressedPairHelp(FF&& first, SS&& second) : F(), S() {
    }
};

template <typename F, typename S>
class CompressedPairHelp<F, S, false, true, false> : public S {
public:
    CompressedPairHelp(){};
    CompressedPairHelp(F&& first, S&& second = S()) : S(), first_(std::forward<F>(first)) {
    }

    CompressedPairHelp(F&& first) : first_(std::forward<F>(first)) {
    }

    F& GetFirst() {
        return first_;
    }

    const F& GetFirst() const {
        return first_;
    };

    S& GetSecond() {
        return *this;
    }

    const S& GetSecond() const {
        return *this;
    };

private:
    F first_;
};

template <typename F, typename S>
class CompressedPairHelp<F, S, true, false, false> : public F {
public:
    CompressedPairHelp(){};
    CompressedPairHelp(F&& first, S&& second) : F(), second_(std::forward<S>(second)) {
    }

    S& GetSecond() {
        return second_;
    }

    const S& GetSecond() const {
        return second_;
    };

private:
    S second_;
};

template <typename F, typename S>
class CompressedPairHelp<F, S, false, false, false> {
public:
    CompressedPairHelp() {
        first_ = F();
        second_ = S();
    };
    template <typename FF, typename SS>
    CompressedPairHelp(FF&& first, SS&& second)
        : first_(std::forward<FF>(first)), second_(std::forward<SS>(second)) {
    }

    F& GetFirst() {
        return first_;
    }

    const F& GetFirst() const {
        return first_;
    };

    S& GetSecond() {
        return second_;
    }

    const S& GetSecond() const {
        return second_;
    };

private:
    S second_;
    F first_;
};

template <typename F, typename S>
class CompressedPairHelp<F, S, false, false, true> {
public:
    template <typename FF, typename SS>
    CompressedPairHelp(FF&& first, SS&& second)
        : first_(std::forward<FF>(first)), second_(std::forward<SS>(second)) {
    }

    F& GetFirst() {
        return first_;
    }

    const F& GetFirst() const {
        return first_;
    };

    S& GetSecond() {
        return second_;
    }

    const S& GetSecond() const {
        return second_;
    };

private:
    S second_;
    F first_;
};

template <typename F, typename S>
class CompressedPair
    : public CompressedPairHelp<F, S, std::is_empty<F>::value && !std::is_final_v<F>,
                                std::is_empty<S>::value && !std::is_final_v<S>,
                                std::is_same<F, S>::value> {
public:
    CompressedPair()
        : CompressedPairHelp<F, S, std::is_empty<F>::value, std::is_empty<S>::value,
                             std::is_same<F, S>::value>(){};
    template <typename FF, typename SS>
    CompressedPair(FF&& first, SS&& second)
        : CompressedPairHelp<F, S, std::is_empty<F>::value, std::is_empty<S>::value,
                             std::is_same<F, S>::value>(std::forward<FF>(first),
                                                        std::forward<SS>(second)) {
    }

    template <typename FF, typename SS>
    CompressedPair(FF&& first)
        : CompressedPairHelp<F, S, std::is_empty<F>::value, std::is_empty<S>::value,
                             std::is_same<F, S>::value>(std::forward<FF>(first)) {
    }

    F& GetFirst() {
        return CompressedPairHelp<F, S, std::is_empty<F>::value, std::is_empty<S>::value,
                                  std::is_same<F, S>::value>::GetFirst();
    }

    const F& GetFirst() const {
        return CompressedPairHelp<F, S, std::is_empty<F>::value, std::is_empty<S>::value,
                                  std::is_same<F, S>::value>::GetFirst();
    };

    S& GetSecond() {
        return CompressedPairHelp<F, S, std::is_empty<F>::value, std::is_empty<S>::value,
                                  std::is_same<F, S>::value>::GetSecond();
    }

    const S& GetSecond() const {
        return CompressedPairHelp<F, S, std::is_empty<F>::value, std::is_empty<S>::value,
                                  std::is_same<F, S>::value>::GetSecond();
    };
};
