#pragma once

#ifndef NO_PCH
#include "../pch.hpp"
#else
#include <cstddef>
#endif

namespace utils {
    /**
     * Get pointer offset by \p offsetBytes from \p ptr.
     * @param ptr Base pointer.
     * @param offsetBytes Offset in bytes to be added to \p ptr. This can be negative.
     * @return Pointer offset by \p offsetBytes from \p ptr.
     */
    [[nodiscard]] void *offsetPtr(void *ptr, std::ptrdiff_t offsetBytes) noexcept {
        return static_cast<void*>(static_cast<std::byte*>(ptr) + offsetBytes);
    }

    /**
     * @copydoc offsetPtr(void*, std::ptrdiff_t)
     */
    [[nodiscard]] const void *offsetPtr(const void *ptr, std::ptrdiff_t offsetBytes) noexcept {
        return static_cast<const void*>(static_cast<const std::byte*>(ptr) + offsetBytes);
    }
}
