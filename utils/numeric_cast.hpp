#pragma once

#ifndef NO_PCH
#include "../pch.hpp"
#else
#include <concepts>
#include <stdexcept>
#include <limits>
#endif

namespace utils {
    template <std::unsigned_integral To>
    [[nodiscard]] To numeric_cast(std::unsigned_integral auto from) {
        if (from > std::numeric_limits<To>::max()) {
            throw std::overflow_error { "Value cannot be represented in the target type" };
        }

        return static_cast<To>(from);
    }
}