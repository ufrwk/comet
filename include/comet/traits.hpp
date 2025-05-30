#pragma once

namespace comet {

template <typename T>
struct remove_cv {
    using type = T;
};
template <typename T>
struct remove_cv<const T> {
    using type = T;
};
template <typename T>
struct remove_cv<volatile T> {
    using type = T;
};
template <typename T>
struct remove_cv<const volatile T> {
    using type = T;
};

template <typename T>
struct remove_reference {
    using type = T;
};
template <typename T>
struct remove_reference<T&> {
    using type = T;
};
template <typename T>
struct remove_reference<T&&> {
    using type = T;
};

template <typename T>
struct is_lvalue_reference {
    static constexpr bool value = false;
};
template <typename T>
struct is_lvalue_reference<T&> {
    static constexpr bool value = true;
};

template <typename T>
constexpr T&& forward(typename remove_reference<T>::type& t) noexcept {
    return static_cast<T&&>(t);
}

template <typename T>
constexpr T&& forward(typename remove_reference<T>::type&& t) noexcept {
    static_assert(!is_lvalue_reference<T>::value, "bad forward of rvalue as lvalue");
    return static_cast<T&&>(t);
}

template <typename T>
struct decay {
    using type = typename remove_cv<typename remove_reference<T>::type>::type;
};

}; // namespace comet
