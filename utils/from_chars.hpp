#pragma once

#ifndef NO_PCH
#include "../pch.hpp"
#else
#include <charconv>
#include <concepts>
#include <expected>
#include <string_view>
#endif

namespace utils {
    template <std::integral T>
    [[nodiscard]] std::expected<T, std::errc> from_chars(std::string_view str, int base = 10) noexcept {
        T result;
        const auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result, base);
        if (ec == std::errc()) {
            return result;
        } else {
            return std::unexpected { ec };
        }
    }
}