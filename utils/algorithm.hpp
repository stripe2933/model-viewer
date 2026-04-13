#pragma once

#ifndef NO_PCH
#include "../pch.hpp"
#else
#include <cassert>
#include <cstdint>
#include <span>
#include <utility>
#endif

namespace utils {
    /**
     * @brief Sort \p data, with key at the corresponding index in \p keys, using bubble sort.
     * @example data = ['a', 'b', 'c', 'd'], keys = [3, 1, 4, 2] -> data will be sorted to ['b', 'd', 'a', 'c'].
     * @param data Data to be sorted.
     * @param keys Keys to sort by. \p data will be sorted according to the order of \p keys. \p data and \p keys must have the same size.
     */
    template <typename T, typename U>
    void bubble_sort_like(std::span<T> data, std::span<U> keys) {
        assert(data.size() == keys.size() && "data and keys must have the same size");

        while (keys.size() >= 2) {
            for (std::size_t i = 0; i + 1 < keys.size(); ++i) {
                if (keys[i] > keys[i + 1]) {
                    std::swap(keys[i], keys[i + 1]);
                    std::swap(data[i], data[i + 1]);
                }
            }
            keys = keys.subspan(0, keys.size() - 1);
        }
    }
}