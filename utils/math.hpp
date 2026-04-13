#pragma once

#ifndef NO_PCH
#include "../pch.hpp"
#else
#include <cassert>
#include <bit>
#include <concepts>
#endif

namespace utils {
    /**
     * Aligns \p size to the next multiple of \p alignment.
     * @example alignedSize(5, 4) -> 8, alignedSize(12, 4) -> 12, alignedSize(13, 4) -> 16.
     * @tparam T Unsigned integral type.
     * @param size Size to be aligned.
     * @param alignment Alignment to align to. Must be a power of two.
     * @return Aligned size.
     */
    template <std::unsigned_integral T>
    [[nodiscard]] constexpr T alignedSize(T size, T alignment) noexcept {
        assert(std::has_single_bit(alignment) && "Alignment must be a power of two");
        return (size + alignment - 1) & ~(alignment - 1);
    }
}