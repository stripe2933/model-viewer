#include "Gpu.hpp"

#include "vku.hpp"

vulkan::Gpu::QueueFamilies::QueueFamilies(const vk::raii::PhysicalDevice &physicalDevice, vk::SurfaceKHR surface)
    : graphicsPresent { 0 } {
    for (const vk::QueueFamilyProperties &prop : physicalDevice.getQueueFamilyProperties()) {
        if (vku::contains(prop.queueFlags, vk::QueueFlagBits::eGraphics) && physicalDevice.getSurfaceSupportKHR(graphicsPresent, surface)) {
            return;
        }

        ++graphicsPresent;
    }

    throw std::runtime_error { "Failed to find the Vulkan queue family that supports both graphics and presentation" };
}

vulkan::Gpu::Gpu(const vk::raii::Instance &instance, vk::SurfaceKHR surface)
    : physicalDevice { instance.enumeratePhysicalDevices().at(0) } // TODO: proper physical device selection
    , queueFamilies { physicalDevice, surface }
    , device { physicalDevice, vk::StructureChain {
        vk::DeviceCreateInfo {
            {},
            vku::lvalue(vk::DeviceQueueCreateInfo {
                {},
                queueFamilies.graphicsPresent,
                vk::ArrayProxyNoTemporaries<const float> { vku::lvalue(1.f) },
            }),
            {},
            vk::KHRSwapchainExtensionName,
        },
        vk::PhysicalDeviceVulkan13Features{}
            .setDynamicRendering(true),
    }.get() }
    , queues {
        .graphicsPresent { (*device).getQueue(queueFamilies.graphicsPresent, 0, *device.getDispatcher()) },
    }
    , allocator { instance, device, vma::AllocatorCreateInfo {
        {},
        *physicalDevice, {},
        {}, {}, {}, {}, {},
        {},
        vk::makeApiVersion(0, 1, 3, 0),
    } } { }