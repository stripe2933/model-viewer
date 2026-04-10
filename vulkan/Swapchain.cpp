#include "Swapchain.hpp"

#ifdef NO_PCH
#include <cstdint>
#include <numeric>

#include <vku.hpp>
#endif

namespace {
    [[nodiscard]] std::uint32_t getMinImageCount(const vk::SurfaceCapabilitiesKHR &surfaceCapabilities) noexcept {
        std::uint32_t result = surfaceCapabilities.minImageCount + 1;
        if (surfaceCapabilities.maxImageCount > 0 && result > surfaceCapabilities.maxImageCount) {
            result = surfaceCapabilities.maxImageCount;
        }
        return result;
    }
}

vulkan::Swapchain::Swapchain(const Gpu &gpu, vk::SurfaceKHR surface, vk::Extent2D extent, vk::SwapchainKHR oldSwapchain)
    : SwapchainKHR { gpu.device, [&] {
        const vk::SurfaceCapabilitiesKHR surfaceCapabilities = gpu.physicalDevice.getSurfaceCapabilitiesKHR(surface);
        extent.width = std::clamp(extent.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
        extent.height = std::clamp(extent.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);

        return vk::SwapchainCreateInfoKHR {
            {},
            surface,
            getMinImageCount(surfaceCapabilities),
            vk::Format::eB8G8R8A8Unorm,
            vk::ColorSpaceKHR::eSrgbNonlinear,
            extent,
            1,
            vk::ImageUsageFlagBits::eColorAttachment,
            vk::SharingMode::eExclusive, {},
            surfaceCapabilities.currentTransform,
            vk::CompositeAlphaFlagBitsKHR::eOpaque,
            vk::PresentModeKHR::eFifo,
            true,
            oldSwapchain,
        };
    }() }
    , extent { extent }
    , images { getImages() } {
    imageViews.reserve(images.size());
    imageReadySemaphores.reserve(images.size());
    for (vk::Image image : images) {
        imageViews.emplace_back(gpu.device, vk::ImageViewCreateInfo {
            {},
            image,
            vk::ImageViewType::e2D,
            vk::Format::eB8G8R8A8Unorm,
            {},
            vku::fullSubresourceRange(vk::ImageAspectFlagBits::eColor),
        });
        imageReadySemaphores.emplace_back(gpu.device, vk::SemaphoreCreateInfo{});
    }
}