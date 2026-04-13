#pragma once

#ifndef NO_PCH
#include "../pch.hpp"
#else
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#include <boost/container/static_vector.hpp>
#include <boost/container/small_vector.hpp>

#include <fastgltf/tools.hpp>

#include <vku.hpp>
#endif

namespace vulkan {
    struct Model {
        struct Mesh {
            struct Attribute {
                std::shared_ptr<vku::raii::AllocatedBuffer> buffer;
                vk::DeviceSize offset;
                vk::DeviceSize stride;
                std::uint32_t count;
                vk::Format format;
            };

            struct Indices {
                std::shared_ptr<vku::raii::AllocatedBuffer> buffer;
                vk::DeviceSize offset;
                std::uint32_t count;
                vk::IndexType type;
            };

            std::optional<Indices> indices;
            Attribute positionAttribute;
            std::optional<Attribute> normalAttribute;
            std::optional<Attribute> tangentAttribute;
            std::optional<Attribute> colorAttribute;
            boost::container::static_vector<Attribute, 4> texcoordAttributes;
            boost::container::small_vector<Attribute, 1> jointsAttributes;
            boost::container::small_vector<Attribute, 1> weightsAttributes;
            vk::PrimitiveTopology topology;
        };

        struct Node {
            std::vector<std::size_t> meshIndices;
            std::optional<vku::raii::AllocatedBuffer> instanceTransformBuffer;
            std::uint32_t instanceCount;
        };

        std::vector<Mesh> meshes;
        std::vector<Node> nodes;
        vku::raii::AllocatedBuffer worldTransformBuffer;

        Model(const fastgltf::Asset &asset, std::size_t sceneIndex, const vma::raii::Allocator &allocator, const fastgltf::DefaultBufferDataAdapter &adapter = {} /* TODO */);
    };
}