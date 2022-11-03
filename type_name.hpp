#pragma once

#include <string_view>

namespace tlib {

namespace detail {

template<typename T>
constexpr std::string_view wrapped_type_name() {
    return __PRETTY_FUNCTION__;
}

}

template <typename T>
constexpr std::string_view type_name() {
    // Using 'void' as known type to find the bounds.
    constexpr auto wrapped_type_name_void = detail::wrapped_type_name<void>();
    constexpr auto prefix_size = wrapped_type_name_void.find("void");
    constexpr auto suffix_size = wrapped_type_name_void.size() - prefix_size - 4;

    std::string_view name = detail::wrapped_type_name<T>();

    name.remove_prefix(prefix_size);
    name.remove_suffix(suffix_size);
    return name;
}

template<typename T>
constexpr std::string_view type_name(T&& t) {
    return type_name<decltype(t)>();
}

}
