#pragma once

#ifndef NO_PCH
#include "../pch.hpp"
#else
#include <cstdint>
#include <vector>

#include <vulkan/vulkan_raii.hpp>
#include <vulkan-memory-allocator-hpp/vk_mem_alloc_raii.hpp>
#endif

namespace vulkan {
    struct Gpu {
        struct QueueFamilies {
            std::uint32_t graphicsPresent;

            QueueFamilies(const vk::raii::PhysicalDevice &physicalDevice, vk::SurfaceKHR surface);
        };

        struct Queues {
            vk::Queue graphicsPresent;
        };

        vk::raii::PhysicalDevice physicalDevice;
        QueueFamilies queueFamilies;
        vk::raii::Device device;
        Queues queues;
        vma::raii::Allocator allocator;

        Gpu(const vk::raii::Instance &instance, vk::SurfaceKHR surface);
    };
}