#pragma once

#ifndef NO_PCH
#include "../pch.hpp"
#else
#include <vector>

#include <vulkan/vulkan_raii.hpp>
#endif

#include "Gpu.hpp"

namespace vulkan {
    struct Swapchain final : vk::raii::SwapchainKHR {
        vk::Extent2D extent;
        std::vector<vk::Image> images;
        std::vector<vk::raii::ImageView> imageViews;
        std::vector<vk::raii::Semaphore> imageReadySemaphores;

        Swapchain(const Gpu &gpu, vk::SurfaceKHR surface, vk::Extent2D extent, vk::SwapchainKHR oldSwapchain = {});
    };
}