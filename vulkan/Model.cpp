#include "Model.hpp"

#ifdef NO_PCH
#include <cstring>
#include <span>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#endif

#include "../utils/algorithm.hpp"
#include "../utils/fastgltf.hpp"
#include "../utils/from_chars.hpp"
#include "../utils/type_variant.hpp"

using namespace std::string_view_literals;

namespace {
    [[nodiscard]] utils::type_variant<std::integral_constant<int, 2>, std::integral_constant<int, 3>, std::integral_constant<int, 4>> getAttributeAccessorTypeVariant(fastgltf::AccessorType type) {
        switch (type) {
            case fastgltf::AccessorType::Vec2:
                return utils::type_variant<std::integral_constant<int, 2>, std::integral_constant<int, 3>, std::integral_constant<int, 4>> { std::type_identity<std::integral_constant<int, 2>>{} };
            case fastgltf::AccessorType::Vec3:
                return utils::type_variant<std::integral_constant<int, 2>, std::integral_constant<int, 3>, std::integral_constant<int, 4>> { std::type_identity<std::integral_constant<int, 3>>{} };
            case fastgltf::AccessorType::Vec4:
                return utils::type_variant<std::integral_constant<int, 2>, std::integral_constant<int, 3>, std::integral_constant<int, 4>> { std::type_identity<std::integral_constant<int, 4>>{} };
            default:
                throw std::invalid_argument { "type: unsupported type for attribute accessor" };
        }
    }

    [[nodiscard]] utils::type_variant<std::int8_t, std::uint8_t, std::int16_t, std::uint16_t, float> getAttributeAccessorComponentTypeVariant(fastgltf::ComponentType type) {
        switch (type) {
            case fastgltf::ComponentType::Byte:
                return utils::type_variant<std::int8_t, std::uint8_t, std::int16_t, std::uint16_t, float> { std::type_identity<std::int8_t>{} };
            case fastgltf::ComponentType::UnsignedByte:
                return utils::type_variant<std::int8_t, std::uint8_t, std::int16_t, std::uint16_t, float> { std::type_identity<std::uint8_t>{} };
            case fastgltf::ComponentType::Short:
                return utils::type_variant<std::int8_t, std::uint8_t, std::int16_t, std::uint16_t, float> { std::type_identity<std::int16_t>{} };
            case fastgltf::ComponentType::UnsignedShort:
                return utils::type_variant<std::int8_t, std::uint8_t, std::int16_t, std::uint16_t, float> { std::type_identity<std::uint16_t>{} };
            case fastgltf::ComponentType::Float:
                return utils::type_variant<std::int8_t, std::uint8_t, std::int16_t, std::uint16_t, float> { std::type_identity<float>{} };
            default:
                throw std::invalid_argument { "type: unsupported component type for attribute accessor" };
        }
    }

    [[nodiscard]] utils::type_variant<std::uint8_t, std::uint16_t, std::uint32_t> getIndicesAccessorComponentType(fastgltf::ComponentType type) {
        switch (type) {
            case fastgltf::ComponentType::UnsignedByte:
                return utils::type_variant<std::uint8_t, std::uint16_t, std::uint32_t> { std::type_identity<std::uint8_t>{} };
            case fastgltf::ComponentType::UnsignedShort:
                return utils::type_variant<std::uint8_t, std::uint16_t, std::uint32_t> { std::type_identity<std::uint16_t>{} };
            case fastgltf::ComponentType::UnsignedInt:
                return utils::type_variant<std::uint8_t, std::uint16_t, std::uint32_t> { std::type_identity<std::uint32_t>{} };
            default:
                throw std::invalid_argument { "type: unsupported component type for indices accessor" };
        }
    }

    [[nodiscard]] vk::PrimitiveTopology getPrimitiveTopology(fastgltf::PrimitiveType type) {
        switch (type) {
            case fastgltf::PrimitiveType::Points:
                return vk::PrimitiveTopology::ePointList;
            case fastgltf::PrimitiveType::Lines:
                return vk::PrimitiveTopology::eLineList;
            case fastgltf::PrimitiveType::LineStrip:
                return vk::PrimitiveTopology::eLineStrip;
            case fastgltf::PrimitiveType::Triangles:
                return vk::PrimitiveTopology::eTriangleList;
            case fastgltf::PrimitiveType::TriangleStrip:
                return vk::PrimitiveTopology::eTriangleStrip;
            case fastgltf::PrimitiveType::TriangleFan:
                return vk::PrimitiveTopology::eTriangleFan;
            default:
                throw std::invalid_argument { "type: unsupported primitive type" };
        }
    }

    [[nodiscard]] vk::Format getAttributeAccessorFloatFormat(fastgltf::AccessorType accessorType, fastgltf::ComponentType componentType, bool normalized) {
        if (componentType == fastgltf::ComponentType::Float && normalized) {
            throw std::invalid_argument { "Float accessor cannot be normalized" };
        }

        switch (accessorType) {
            case fastgltf::AccessorType::Vec2:
                // TEXCOORD_n
                switch (componentType) {
                    case fastgltf::ComponentType::Byte:
                        return normalized ? vk::Format::eR8G8Snorm : vk::Format::eR8G8Sscaled;
                    case fastgltf::ComponentType::UnsignedByte:
                        return normalized ? vk::Format::eR8G8Unorm : vk::Format::eR8G8Uscaled;
                    case fastgltf::ComponentType::Short:
                        return normalized ? vk::Format::eR16G16Snorm : vk::Format::eR16G16Sscaled;
                    case fastgltf::ComponentType::UnsignedShort:
                        return normalized ? vk::Format::eR16G16Unorm : vk::Format::eR16G16Uscaled;
                    case fastgltf::ComponentType::Float:
                        return vk::Format::eR32G32Sfloat;
                    default:
                        break;
                }
                break;
            case fastgltf::AccessorType::Vec3:
                // POSITION, NORMAL, COLOR_n, TANGENT morph target
                switch (componentType) {
                    case fastgltf::ComponentType::Byte:
                        return normalized ? vk::Format::eR8G8B8Snorm : vk::Format::eR8G8B8Sscaled;
                    case fastgltf::ComponentType::UnsignedByte:
                        return normalized ? vk::Format::eR8G8B8Unorm : vk::Format::eR8G8B8Uscaled;
                    case fastgltf::ComponentType::Short:
                        return normalized ? vk::Format::eR16G16B16Snorm : vk::Format::eR16G16B16Sscaled;
                    case fastgltf::ComponentType::UnsignedShort:
                        return normalized ? vk::Format::eR16G16B16Unorm : vk::Format::eR16G16B16Uscaled;
                    case fastgltf::ComponentType::Float:
                        return vk::Format::eR32G32B32Sfloat;
                    default:
                        break;
                }
                break;
            case fastgltf::AccessorType::Vec4:
                // TANGENT, COLOR_n, WEIGHTS_n
                if (componentType != fastgltf::ComponentType::Float && !normalized) {
                    throw std::invalid_argument { "Only TANGENT and COLOR_n attributes are allowed for VEC4 (unsigned){byte,short} and they must be normalized" };
                }

                switch (componentType) {
                    case fastgltf::ComponentType::Byte:
                        return vk::Format::eR8G8B8A8Snorm;
                    case fastgltf::ComponentType::UnsignedByte:
                        return vk::Format::eR8G8B8A8Unorm;
                    case fastgltf::ComponentType::Short:
                        return vk::Format::eR16G16B16A16Snorm;
                    case fastgltf::ComponentType::UnsignedShort:
                        return vk::Format::eR16G16B16A16Unorm;
                    case fastgltf::ComponentType::Float:
                        return vk::Format::eR32G32B32A32Sfloat;
                    default:
                        break;
                }
                break;
            default:
                break;
        }

        throw std::invalid_argument { "Unknown (accessorType, componentType, normalized) combination" };
    }

    [[nodiscard]] vk::Format getAttributeAccessorIntFormat(fastgltf::AccessorType accessorType, fastgltf::ComponentType componentType, bool normalized) {
        if (accessorType != fastgltf::AccessorType::Vec4 || normalized) {
            throw std::invalid_argument { "JOINTS_n attribute must be VEC4 type and not be normalized" };
        }

        switch (componentType) {
            case fastgltf::ComponentType::UnsignedByte:
                return vk::Format::eR8G8B8A8Uint;
            case fastgltf::ComponentType::UnsignedShort:
                return vk::Format::eR16G16B16A16Uint;
            default:
                throw std::invalid_argument { "Unknown (accessorType, componentType, normalized) combination" };
        }
    }
}

vulkan::Model::Model(const fastgltf::Asset &asset, std::size_t sceneIndex, const vma::raii::Allocator &allocator, const fastgltf::DefaultBufferDataAdapter &adapter)
    : worldTransformBuffer {
        allocator,
        vk::BufferCreateInfo {
            {},
            asset.nodes.size() * sizeof(fastgltf::math::fmat4x4),
            vk::BufferUsageFlagBits::eStorageBuffer,
        },
        vma::AllocationCreateInfo {
            vma::AllocationCreateFlagBits::eHostAccessRandom | vma::AllocationCreateFlagBits::eMapped,
            vma::MemoryUsage::eAutoPreferHost,
        },
    } {
    constexpr vk::BufferUsageFlags attributeBufferUsageFlags = vk::BufferUsageFlagBits::eVertexBuffer;
    constexpr vk::BufferUsageFlags indexBufferUsageFlags = vk::BufferUsageFlagBits::eIndexBuffer;

    std::unordered_set<std::size_t> attributeAccessors;
    std::unordered_set<std::size_t> indicesAccessors;
    std::unordered_map<std::size_t, vk::BufferUsageFlags> bufferViewBufferUsages;
    vk::BufferUsageFlags zeroBufferUsage{};

    std::unordered_map<const fastgltf::Primitive*, std::size_t> meshIndicesByPrimitive;
    for (const fastgltf::Mesh &mesh : asset.meshes) {
        for (const fastgltf::Primitive &primitive : mesh.primitives) {
            const auto positionAttributeIt = primitive.findAttribute("POSITION");
            if (positionAttributeIt == primitive.attributes.end()) {
                // glTF 2.0 specification (3.7.2. Meshes)
                //
                // When positions are not specified, client implementations SHOULD skip primitive’s rendering unless its
                // positions are provided by other means (e.g., by an extension). This applies to both indexed and
                // non-indexed geometry.
                continue;
            }

            const auto collectAttributeBufferUsages = [&](std::size_t accessorIndex) {
                const fastgltf::Accessor &accessor = asset.accessors[accessorIndex];
                if (accessor.sparse) {
                    attributeAccessors.emplace(accessorIndex);
                }
                else if (!accessor.bufferViewIndex) {
                    zeroBufferUsage |= attributeBufferUsageFlags;
                }
                else {
                    bufferViewBufferUsages[*accessor.bufferViewIndex] |= attributeBufferUsageFlags;
                }
            };

            collectAttributeBufferUsages(positionAttributeIt->accessorIndex);

            std::optional<std::size_t> deferredTangentAttributeAccessorIndex;
            for (const auto &[attributeName, accessorIndex] : primitive.attributes) {
                bool collect = false;
                if (attributeName == "NORMAL"sv) {
                    collect = true;

                    // See the below TANGENT attribute collection defer.
                    if (deferredTangentAttributeAccessorIndex) {
                        collectAttributeBufferUsages(*deferredTangentAttributeAccessorIndex);
                    }
                }
                else if (attributeName == "TANGENT"sv) {
                    // glTF 2.0 specification (3.7.2. Meshes)
                    //
                    // When normals are not specified, client implementations MUST calculate flat normals and the
                    // provided tangents (if present) MUST be ignored.
                    //
                    // Therefore, collecting the TANGENT attribute accessor will be deferred until NORMAL attribute found.
                    deferredTangentAttributeAccessorIndex.emplace(accessorIndex);
                }
                else if (attributeName == "COLOR_0"sv || attributeName.starts_with("JOINTS_"sv) || attributeName.starts_with("WEIGHTS_"sv)) {
                    collect = true;
                }
                else if (attributeName.starts_with("TEXCOORD_"sv) &&
                    utils::from_chars<std::size_t>(std::string_view { attributeName }.substr(9)).value() < 4) {
                    collect = true;
                }

                if (collect) {
                    collectAttributeBufferUsages(accessorIndex);
                }
            }

            if (primitive.indicesAccessor) {
                const fastgltf::Accessor &accessor = asset.accessors[*primitive.indicesAccessor];
                if (accessor.sparse) {
                    indicesAccessors.emplace(*primitive.indicesAccessor);
                }
                else if (!accessor.bufferViewIndex) {
                    zeroBufferUsage |= indexBufferUsageFlags;
                }
                else {
                    bufferViewBufferUsages[*accessor.bufferViewIndex] |= indexBufferUsageFlags;
                }
            }

            meshIndicesByPrimitive.emplace(&primitive, meshIndicesByPrimitive.size());
        }
    }

    std::unordered_map<std::size_t, std::shared_ptr<vku::raii::AllocatedBuffer>> attributeAccessorBuffers;
    for (std::size_t accessorIndex : attributeAccessors) {
        const fastgltf::Accessor &accessor = asset.accessors[accessorIndex];

        const std::size_t elementByteSize = getElementByteSize(accessor.type, accessor.componentType);
        const vk::DeviceSize stride = vku::alignedSize<vk::DeviceSize>(elementByteSize, 4);

        auto &buffer = attributeAccessorBuffers.emplace(
            accessorIndex,
            std::make_shared<vku::raii::AllocatedBuffer>(
                allocator,
                vk::BufferCreateInfo {
                    {},
                    // In the shader, attribute data whose size is less than 4 will be loaded as 4-byte uint, unpacked
                    // to the quantized vector, then only the relevant components will be extracted. Therefore, the
                    // buffer size must be a multiple of 4.
                    vku::alignedSize<vk::DeviceSize>(stride * (accessor.count - 1) + elementByteSize, 4),
                    attributeBufferUsageFlags,
                },
                vma::AllocationCreateInfo {
                    vma::AllocationCreateFlagBits::eHostAccessSequentialWrite | vma::AllocationCreateFlagBits::eMapped,
                    vma::MemoryUsage::eAutoPreferHost,
                })).first->second;

        std::visit([&]<int N, typename T>(std::type_identity<std::integral_constant<int, N>>, std::type_identity<T>) {
            using ElementType = fastgltf::math::vec<T, N>;
            constexpr std::size_t Stride = vku::alignedSize(sizeof(ElementType), 4UZ);
            copyFromAccessor<ElementType, Stride>(asset, accessor, buffer->getAllocation().getInfo().pMappedData);
        }, getAttributeAccessorTypeVariant(accessor.type).to_variant(), getAttributeAccessorComponentTypeVariant(accessor.componentType).to_variant());
        buffer->getAllocation().flush(0, vk::WholeSize);
    }

    std::unordered_map<std::size_t, std::shared_ptr<vku::raii::AllocatedBuffer>> indicesAccessorBuffers;
    for (std::size_t accessorIndex : indicesAccessors) {
        const fastgltf::Accessor &accessor = asset.accessors[accessorIndex];

        const vk::DeviceSize componentByteSize = getComponentByteSize(accessor.componentType);

        auto &buffer = indicesAccessorBuffers.emplace(
            accessorIndex,
            std::make_shared<vku::raii::AllocatedBuffer>(
                allocator,
                vk::BufferCreateInfo {
                    {},
                    componentByteSize * accessor.count,
                    indexBufferUsageFlags,
                },
                vma::AllocationCreateInfo {
                    vma::AllocationCreateFlagBits::eHostAccessSequentialWrite | vma::AllocationCreateFlagBits::eMapped,
                    vma::MemoryUsage::eAutoPreferHost,
                })).first->second;

        getIndicesAccessorComponentType(accessor.componentType).visit([&]<typename T>(std::type_identity<T>) {
            copyFromAccessor<T>(asset, accessor, buffer->getAllocation().getInfo().pMappedData);
        });
        buffer->getAllocation().flush(0, vk::WholeSize);
    }

    std::unordered_map<std::size_t, std::shared_ptr<vku::raii::AllocatedBuffer>> bufferViewBuffers;
    for (const auto &[bufferViewIndex, usageFlags] : bufferViewBufferUsages) {
        const fastgltf::BufferView &bufferView = asset.bufferViews[bufferViewIndex];
        auto &buffer = bufferViewBuffers.emplace(
            bufferViewIndex,
            std::make_shared<vku::raii::AllocatedBuffer>(
                allocator,
                vk::BufferCreateInfo {
                    {},
                    bufferView.byteLength,
                    usageFlags,
                },
                vma::AllocationCreateInfo {
                    vma::AllocationCreateFlagBits::eHostAccessSequentialWrite | vma::AllocationCreateFlagBits::eMapped,
                    vma::MemoryUsage::eAutoPreferHost,
                })).first->second;


        const std::span<const std::byte> bufferViewBytes = adapter(asset, bufferViewIndex);
        buffer->getAllocation().copyFromMemory(bufferViewBytes.data(), 0, bufferViewBytes.size_bytes());
    }

    std::shared_ptr<vku::raii::AllocatedBuffer> zeroBuffer = nullptr;
    if (zeroBufferUsage != vk::BufferUsageFlags{}) {
        zeroBuffer = std::make_shared<vku::raii::AllocatedBuffer>(
            allocator,
            vk::BufferCreateInfo {
                {},
                16,
                zeroBufferUsage,
            },
            vma::AllocationCreateInfo {
                vma::AllocationCreateFlagBits::eHostAccessSequentialWrite | vma::AllocationCreateFlagBits::eMapped,
                vma::MemoryUsage::eAutoPreferDevice,
            });
        std::memset(zeroBuffer->getAllocation().getInfo().pMappedData, 0, 16);
        zeroBuffer->getAllocation().flush(0, vk::WholeSize);
    }

    meshes.resize(meshIndicesByPrimitive.size());
    for (const auto &[primitive, meshIndex] : meshIndicesByPrimitive) {
        Mesh &modelMesh = meshes[meshIndex];
        modelMesh.topology = getPrimitiveTopology(primitive->type);

        std::optional<std::size_t> deferredTangentAttributeAccessorIndex;
        boost::container::static_vector<std::size_t, 4> texcoordSortArgs;
        boost::container::small_vector<std::size_t, 1> jointsSortArgs;
        boost::container::small_vector<std::size_t, 1> weightsSortArgs;
        for (const auto &[attributeName, accessorIndex] : primitive->attributes) {
            const auto createAttribute = [&](std::size_t accessorIndex, bool joint) -> Mesh::Attribute {
                const fastgltf::Accessor &accessor = asset.accessors[accessorIndex];
                const vk::Format format = [&] {
                    if (joint) {
                        return getAttributeAccessorIntFormat(accessor.type, accessor.componentType, accessor.normalized);
                    }
                    else {
                        return getAttributeAccessorFloatFormat(accessor.type, accessor.componentType, accessor.normalized);
                    }
                }();

                if (accessor.sparse) {
                    return {
                        .buffer = attributeAccessorBuffers.at(accessorIndex),
                        .offset = 0,
                        .stride = vku::alignedSize<vk::DeviceSize>(getElementByteSize(accessor.type, accessor.componentType), 4),
                        .count = static_cast<std::uint32_t>(accessor.count),
                        .format = format,
                    };
                }
                else if (!accessor.bufferViewIndex) {
                    return {
                        .buffer = zeroBuffer,
                        .offset = 0,
                        .stride = 0,
                        .count = static_cast<std::uint32_t>(accessor.count),
                        .format = format,
                    };
                }
                else {
                    const fastgltf::BufferView &bufferView = asset.bufferViews[*accessor.bufferViewIndex];
                    return {
                        .buffer = bufferViewBuffers.at(*accessor.bufferViewIndex),
                        .offset = accessor.byteOffset,
                        .stride = bufferView.byteStride.value_or(getElementByteSize(accessor.type, accessor.componentType)),
                        .count = static_cast<std::uint32_t>(accessor.count),
                        .format = format,
                    };
                }
            };

            if (attributeName == "POSITION"sv) {
                modelMesh.positionAttribute = createAttribute(accessorIndex, false);
            }
            else if (attributeName == "NORMAL"sv) {
                modelMesh.normalAttribute.emplace(createAttribute(accessorIndex, false));

                if (deferredTangentAttributeAccessorIndex) {
                    modelMesh.tangentAttribute.emplace(createAttribute(*deferredTangentAttributeAccessorIndex, false));
                }
            }
            else if (attributeName == "TANGENT"sv) {
                deferredTangentAttributeAccessorIndex.emplace(accessorIndex);
            }
            else if (attributeName == "COLOR_0"sv) {
                modelMesh.colorAttribute.emplace(createAttribute(accessorIndex, false));
            }
            else if (attributeName.starts_with("TEXCOORD_")) {
                const std::size_t texcoordIndex = utils::from_chars<std::size_t>(std::string_view { attributeName }.substr(9)).value();
                if (texcoordIndex < 4) {
                    modelMesh.texcoordAttributes.push_back(createAttribute(accessorIndex, false));
                    texcoordSortArgs.push_back(texcoordIndex);
                }
            }
            else if (attributeName.starts_with("JOINTS_")) {
                const std::size_t jointIndex = utils::from_chars<std::size_t>(std::string_view { attributeName }.substr(7)).value();
                modelMesh.jointsAttributes.push_back(createAttribute(accessorIndex, true));
                jointsSortArgs.push_back(jointIndex);
            }
            else if (attributeName.starts_with("WEIGHTS_")) {
                const std::size_t weightIndex = utils::from_chars<std::size_t>(std::string_view { attributeName }.substr(8)).value();
                modelMesh.weightsAttributes.push_back(createAttribute(accessorIndex, false));
                weightsSortArgs.push_back(weightIndex);
            }
        }

        utils::bubble_sort_like(std::span { modelMesh.texcoordAttributes }, std::span { texcoordSortArgs });
        utils::bubble_sort_like(std::span { modelMesh.jointsAttributes }, std::span { jointsSortArgs });
        utils::bubble_sort_like(std::span { modelMesh.weightsAttributes }, std::span { weightsSortArgs });
    }

    nodes.resize(asset.nodes.size());
    for (std::size_t nodeIndex = 0; nodeIndex < asset.nodes.size(); ++nodeIndex) {
        const fastgltf::Node &node = asset.nodes[nodeIndex];
        Node &modelNode = nodes[nodeIndex];

        if (node.meshIndex) {
            const fastgltf::Mesh &mesh = asset.meshes[*node.meshIndex];
            for (const fastgltf::Primitive &primitive : mesh.primitives) {
                if (auto it = meshIndicesByPrimitive.find(&primitive); it != meshIndicesByPrimitive.end()) {
                    modelNode.meshIndices.push_back(it->second);
                }
            }
        }

        if (!node.instancingAttributes.empty()) {
            const std::vector transforms = getInstanceTransforms(asset, nodeIndex, adapter);
            auto &buffer = modelNode.instanceTransformBuffer.emplace(
                allocator,
                vk::BufferCreateInfo {
                    {},
                    transforms.size() * sizeof(fastgltf::math::fmat4x4),
                    vk::BufferUsageFlagBits::eVertexBuffer,
                },
                vma::AllocationCreateInfo {
                    vma::AllocationCreateFlagBits::eHostAccessSequentialWrite | vma::AllocationCreateFlagBits::eMapped,
                    vma::MemoryUsage::eAutoPreferHost,
                });
            buffer.getAllocation().copyFromMemory(transforms.data(), 0, transforms.size() * sizeof(fastgltf::math::fmat4x4));

            modelNode.instanceCount = transforms.size();
        }
    }

    traverseScene(asset, sceneIndex, [this](std::size_t nodeIndex, const fastgltf::math::fmat4x4 &worldTransform) {
        worldTransformBuffer.getAllocation().copyFromMemory(&worldTransform, nodeIndex * sizeof(fastgltf::math::fmat4x4), sizeof(worldTransform));
    });
}