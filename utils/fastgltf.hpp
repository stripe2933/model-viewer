#pragma once

#ifndef NO_PCH
#include "../pch.hpp"
#else
#include <cstdint>
#include <concepts>
#include <numeric>
#include <type_traits>
#include <vector>

#include <fastgltf/types.hpp>
#endif

#define INDEX_SEQ(Is, N, ...) [&]<auto... Is>(std::index_sequence<Is...>) __VA_ARGS__ (std::make_index_sequence<N>{})

namespace fastgltf {
    /**
     * Traverse node's descendants using preorder traversal.
     * @tparam F Function type that can be executed with node index. If it returns contextually convertible to <tt>bool</tt> type, the return value will be determined as the traversal continuation (<tt>true</tt> -> continue traversal).
     * @param asset fastgltf Asset.
     * @param scene Node index to start the traversal.
     * @param f Function that would be invoked with node index.
     */
    template <std::invocable<std::size_t> F>
    void traverseNode(const Asset &asset, std::size_t nodeIndex, F &&f) noexcept(std::is_nothrow_invocable_v<F, std::size_t>) {
        // If F is predicate, traversal continuation is determined by the return value of f.
        if constexpr (std::predicate<F, std::size_t>) {
            // Stop traversal if f returns false.
            if (!f(nodeIndex)) return;
        }
        else {
            f(nodeIndex);
        }

        for (std::size_t childNodeIndex : asset.nodes[nodeIndex].children) {
            traverseNode(asset, childNodeIndex, f);
        }
    }

    /**
     * Traverse node's descendants with accumulated transforms (i.e. world transform) using preorder traversal.
     * @tparam F Function type that can be executed with node index and <tt>fastgltf::math::mat<T, 4, 4></tt>. If it returns contextually convertible to <tt>bool</tt> type, the return value will be determined as the traversal continuation (<tt>true</tt> -> continue traversal).
     * @param asset fastgltf Asset.
     * @param nodeIndex Node index to start the traversal.
     * @param worldTransform World transform matrix of the start node.
     * @param f Function that would be invoked with node index and <tt>fastgltf::math::mat<T, 4, 4></tt>.
     */
    template <std::floating_point T = float, std::invocable<std::size_t, const math::mat<T, 4, 4>&> F>
    void traverseNode(const Asset &asset, std::size_t nodeIndex, const math::mat<T, 4, 4> &worldTransform, F &&f) noexcept(std::is_nothrow_invocable_v<F, std::size_t, const math::mat<T, 4, 4>&>) {
        // If F is predicate, traversal continuation is determined by the return value of f.
        if constexpr (std::predicate<F, std::size_t, const math::mat<T, 4, 4>&>) {
            // Stop traversal if f returns false.
            if (!f(nodeIndex, worldTransform)) return;
        }
        else {
            f(nodeIndex, worldTransform);
        }

        for (std::size_t childNodeIndex : asset.nodes[nodeIndex].children) {
            const math::mat<T, 4, 4> childNodeWorldTransform = worldTransform * cast<T>(getTransformMatrix(asset.nodes[childNodeIndex]));
            traverseNode(asset, childNodeIndex, childNodeWorldTransform, f);
        }
    }

    /**
     * Traverse \p scene using preorder traversal.
     * @tparam F Function type that can be executed with node index. If it returns contextually convertible to <tt>bool</tt> type, the return value will be determined as the traversal continuation (<tt>true</tt> -> continue traversal).
     * @param asset fastgltf Asset.
     * @param sceneIndex Index of the scene to be traversed.
     * @param f Function that would be invoked with node index.
     */
    template <std::invocable<std::size_t> F>
    void traverseScene(const Asset &asset, std::size_t sceneIndex, F &&f) noexcept(std::is_nothrow_invocable_v<F, std::size_t>) {
        for (std::size_t nodeIndex : asset.scenes[sceneIndex].nodeIndices) {
            traverseNode(asset, nodeIndex, f);
        }
    }

    /**
     * Traverse \p scene with accumulated transforms (i.e. world transform) using preorder traversal.
     * @tparam F Function type that can be executed with node index and <tt>fastgltf::math::mat<T, 4, 4></tt>. If it returns contextually convertible to <tt>bool</tt> type, the return value will be determined as the traversal continuation (<tt>true</tt> -> continue traversal).
     * @param asset fastgltf Asset.
     * @param sceneIndex Index of the scene to be traversed.
     * @param f Function that would be invoked with node index and <tt>fastgltf::math::mat<T, 4, 4></tt>.
     */
    template <std::floating_point T = float, std::invocable<std::size_t, const math::mat<T, 4, 4>&> F>
    void traverseScene(const Asset &asset, std::size_t sceneIndex, F &&f) noexcept(std::is_nothrow_invocable_v<F, std::size_t, const math::mat<T, 4, 4>&>) {
        for (std::size_t nodeIndex : asset.scenes[sceneIndex].nodeIndices) {
            traverseNode(asset, nodeIndex, cast<T>(getTransformMatrix(asset.nodes[nodeIndex])), f);
        }
    }

    [[nodiscard]] std::size_t getInstanceCount(const Asset &asset, std::size_t nodeIndex) {
        // According to the EXT_mesh_gpu_instancing specification, all attribute accessors in a given node must
        // have the same count. Therefore, we can use the count of the first attribute accessor.
        // std::out_of_range in here means the node is not instanced.
        return asset.accessors[asset.nodes[nodeIndex].instancingAttributes.at(0).accessorIndex].count;
    }

    /**
     * @brief Get transform matrices of \p node instances.
     *
     * @tparam BufferDataAdapter A functor type that acquires the binary buffer data from a glTF buffer view.
     * @param asset fastgltf asset.
     * @param nodeIndex Node index to get the instance transforms.
     * @param adapter Buffer data adapter. If you provided <tt>fastgltf::Options::LoadExternalBuffers</tt> to the <tt>fastgltf::Parser</tt> while loading the glTF, the parameter can be omitted.
     * @return A vector of instance transform matrices.
     * @throw std::out_of_range If the node is not instanced.
     * @note This function has effect only if \p asset is loaded with EXT_mesh_gpu_instancing extension supporting parser (otherwise, it will return the empty vector).
     */
    template <typename BufferDataAdapter = DefaultBufferDataAdapter>
    [[nodiscard]] std::vector<math::fmat4x4> getInstanceTransforms(const Asset &asset, std::size_t nodeIndex, const BufferDataAdapter &adapter = {}) {
        const std::size_t instanceCount = getInstanceCount(asset, nodeIndex);
        std::vector<math::fmat4x4> result(instanceCount, math::fmat4x4 { 1.f });

        const Node &node = asset.nodes[nodeIndex];
        if (auto it = node.findInstancingAttribute("TRANSLATION"); it != node.instancingAttributes.end()) {
            const Accessor &accessor = asset.accessors[it->accessorIndex];
            iterateAccessorWithIndex<math::fvec3>(asset, accessor, [&](const math::fvec3 &translation, std::size_t i) {
                result[i] = translate(result[i], translation);
            }, adapter);
        }

        if (auto it = node.findInstancingAttribute("ROTATION"); it != node.instancingAttributes.end()) {
            const Accessor &accessor = asset.accessors[it->accessorIndex];
            iterateAccessorWithIndex<math::fquat>(asset, accessor, [&](math::fquat rotation, std::size_t i) {
                result[i] = rotate(result[i], rotation);
            }, adapter);
        }

        if (auto it = node.findInstancingAttribute("SCALE"); it != node.instancingAttributes.end()) {
            const Accessor &accessor = asset.accessors[it->accessorIndex];
            iterateAccessorWithIndex<math::fvec3>(asset, accessor, [&](const math::fvec3 &scale, std::size_t i) {
                result[i] = math::scale(result[i], scale);
            }, adapter);
        }

        return result;
    }

namespace math {
    template <typename T, std::size_t N>
    [[nodiscard]] vec<T, N> operator/(T num, vec<T, N> denom) noexcept {
        INDEX_SEQ(Is, N, {
            ((denom.data()[Is] = num / denom.data()[Is]), ...);
        });
        return denom;
    }

    /**
     * @brief Get component-wise minimum of two vectors.
     * @tparam T Vector component type.
     * @tparam N Number of vector components.
     * @param lhs
     * @param rhs
     * @return Component-wise minimum of two vectors.
     */
    template <typename T, std::size_t N>
    [[nodiscard]] vec<T, N> min(vec<T, N> lhs, const vec<T, N> &rhs) noexcept {
        INDEX_SEQ(Is, N, {
            ((lhs.data()[Is] = std::min(lhs.data()[Is], rhs.data()[Is])), ...);
        });
        return lhs;
    }

    /**
     * @brief Get component-wise maximum of two vectors.
     * @tparam T Vector component type.
     * @tparam N Number of vector components.
     * @param lhs
     * @param rhs
     * @return Component-wise maximum of two vectors.
     */
    template <typename T, std::size_t N>
    [[nodiscard]] vec<T, N> max(vec<T, N> lhs, const vec<T, N> &rhs) noexcept {
        INDEX_SEQ(Is, N, {
            ((lhs.data()[Is] = std::max(lhs.data()[Is], rhs.data()[Is])), ...);
        });
        return lhs;
    }

    template <typename T, std::size_t N>
    [[nodiscard]] std::pair<vec<T, N>, vec<T, N>> minmax(const vec<T, N> &lhs, const vec<T, N> &rhs) noexcept {
        return { min(lhs, rhs), max(lhs, rhs) };
    }

    template <typename T, std::size_t N>
    [[nodiscard]] T compMin(const vec<T, N> &v) noexcept {
        return INDEX_SEQ(Is, N, {
            return std::min({ v.data()[Is]... });
        });
    }

    template <typename T, std::size_t N>
    [[nodiscard]] T compMax(const vec<T, N> &v) noexcept {
        return INDEX_SEQ(Is, N, {
            return std::max({ v.data()[Is]... });
        });
    }

    template <typename T, typename U, std::size_t N>
    [[nodiscard]] constexpr vec<T, N> cast(const vec<U, N> &v) noexcept {
        return INDEX_SEQ(Is, N, {
            return vec<T, N> { static_cast<T>(v.data()[Is])... };
        });
    }

    template <typename T, typename U, std::size_t N, std::size_t M>
    [[nodiscard]] constexpr mat<T, N, M> cast(const mat<U, N, M> &v) noexcept {
        return INDEX_SEQ(Is, N, {
            return mat<T, N, M> { cast<T>(v[Is])... };
        });
    }
}
}
