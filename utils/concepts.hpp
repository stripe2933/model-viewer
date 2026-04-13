#pragma once

#ifndef NO_PCH
#include "../pch.hpp"
#else
#include <concepts>
#endif

namespace concepts {
    template <typename T, typename... Ts>
    concept any_of = (std::same_as<T, Ts> || ...);
}