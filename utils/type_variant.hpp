#pragma once

#ifndef NO_PCH
#include "../pch.hpp"
#else
#include <cstdint>
#include <concepts>
#include <functional>
#include <limits>
#include <type_traits>
#include <utility>
#endif

#include "concepts.hpp"

#define FWD(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)
#define INDEX_SEQ(Is, N, ...) [&]<auto... Is>(std::index_sequence<Is...>) __VA_ARGS__ (std::make_index_sequence<N>{})

namespace utils {
namespace details {
    template <typename T, typename THead, typename... Ts>
    [[nodiscard]] consteval std::uint8_t find_type_index() noexcept {
        if constexpr (std::same_as<T, THead>) {
            return 0;
        }
        else {
            return find_type_index<T, Ts...>() + 1;
        }
    }
}

    template <typename... Ts>
        requires (sizeof...(Ts) < std::numeric_limits<std::uint8_t>::max()
            && (!std::same_as<Ts, std::monostate> && ...))
    class type_variant {
    public:
        explicit type_variant() noexcept
            : index { 0 } { }

        template <concepts::any_of<Ts...> T>
        explicit type_variant(std::type_identity<T>) noexcept
            : index { details::find_type_index<T, Ts...>() } { }

        template <concepts::any_of<Ts...> T>
        void emplace() noexcept {
            index = details::find_type_index<T, Ts...>();
        }

        template <typename F>
        void visit(F &&visitor) noexcept((std::is_nothrow_invocable_v<F, std::type_identity<Ts>> && ...)) {
            INDEX_SEQ(Is, sizeof...(Ts), {
                std::ignore = ((index == Is ? false : (std::invoke(FWD(visitor), std::type_identity<Ts>{}), true)) && ...);
            });
        }

        [[nodiscard]] std::variant<std::type_identity<Ts>...> to_variant() noexcept {
            std::variant<std::type_identity<Ts>...> result;
            visit([&]<typename T>(std::type_identity<T>) noexcept {
                result.template emplace<std::type_identity<T>>();
            });
            return result;
        }

    private:
        std::uint8_t index;
    };
}