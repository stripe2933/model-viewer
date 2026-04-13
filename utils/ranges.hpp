#pragma once

#ifndef NO_PCH
#include "../pch.hpp"
#else
#include <algorithm>
#include <initializer_list>
#endif

namespace ranges {
    template <typename T>
    [[nodiscard]] constexpr bool any_of(const T &value, std::initializer_list<T> candidates) {
        return std::ranges::contains(candidates, value);
    }
}