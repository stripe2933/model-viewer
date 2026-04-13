#ifndef NO_PCH
#include "pch.hpp"
#else
#include <cstdint>
#include <algorithm>
#include <stdexcept>
#include <vector>

#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <vku.hpp>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#undef MemoryBarrier
#endif
#endif

#include "utils/numeric_cast.hpp"
#include "vulkan/Gpu.hpp"
#include "vulkan/Swapchain.hpp"

namespace {
    [[nodiscard]] vk::Extent2D getFramebufferExtent(GLFWwindow *window) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        return { static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height) };
    }
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
#else
int main() {
#endif
    if (!glfwInit()) {
        throw std::runtime_error { "Failed to initialize GLFW" };
    }

    const float mainScale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow *window = glfwCreateWindow(static_cast<int>(1280 * mainScale), static_cast<int>(720 * mainScale), "Model Viewer", nullptr, nullptr);
    if (!window) {
        throw std::runtime_error { "Failed to create GLFW window" };
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGuiStyle &style = ImGui::GetStyle();
    style.ScaleAllSizes(mainScale);
    style.FontScaleDpi = mainScale;

    ImGui_ImplGlfw_InitForVulkan(window, true);

    {
        vk::raii::Context context;

        std::uint32_t glfwExtensionCount;
        const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        const vk::raii::Instance instance { context, vk::InstanceCreateInfo {
            {},
            &vku::lvalue(vk::ApplicationInfo {
                "Model Viewer", 0,
                nullptr, 0,
                vk::makeApiVersion(0, 1, 3, 0),
            }),
            {},
            vk::ArrayProxyNoTemporaries<const char* const> { glfwExtensionCount, glfwExtensions },
        } };

        const vk::raii::SurfaceKHR surface { instance, [&] {
            if (VkSurfaceKHR result; glfwCreateWindowSurface(*instance, window, nullptr, &result) == VK_SUCCESS) {
                return result;
            }
            throw std::runtime_error { "Failed to create Vulkan surface from GLFW window" };
        }() };

        const vulkan::Gpu gpu { instance, surface };

        vulkan::Swapchain swapchain { gpu, surface, getFramebufferExtent(window) };

        constexpr vk::Format colorAttachmentFormat = vk::Format::eB8G8R8A8Unorm;
        ImGui_ImplVulkan_InitInfo initInfo {
            .ApiVersion = vk::makeApiVersion(0, 1, 3, 0),
            .Instance = *instance,
            .PhysicalDevice = *gpu.physicalDevice,
            .Device = *gpu.device,
            .QueueFamily = gpu.queueFamilies.graphicsPresent,
            .Queue = gpu.queues.graphicsPresent,
            .DescriptorPoolSize = 128,
            .MinImageCount = 2,
            .ImageCount = 2,
            .PipelineInfoMain = {
                .PipelineRenderingCreateInfo = vk::PipelineRenderingCreateInfo {
                    {},
                    colorAttachmentFormat,
                },
            },
            .UseDynamicRendering = true,
        };
        ImGui_ImplVulkan_Init(&initInfo);
        
        vk::raii::CommandPool commandPool { gpu.device, vk::CommandPoolCreateInfo {
            {},
            gpu.queueFamilies.graphicsPresent,
        } };
        vk::CommandBuffer commandBuffer = (*gpu.device).allocateCommandBuffers({
            *commandPool,
            vk::CommandBufferLevel::ePrimary,
            1,
        }, *gpu.device.getDispatcher())[0];
        
        vk::raii::Semaphore swapchainImageAcquireSemaphore { gpu.device, vk::SemaphoreCreateInfo{} };
        vk::raii::Fence frameReadyFence { gpu.device, vk::FenceCreateInfo{} };

        auto glfwUserData = std::tie(gpu, surface, swapchain);
        glfwSetWindowUserPointer(window, &glfwUserData);
        glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int width, int height) {
            vk::Extent2D extent { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
            while (width == 0 || height == 0) {
                if (glfwWindowShouldClose(window)) {
                    return;
                }

                extent = getFramebufferExtent(window);
                glfwWaitEvents();
            }

            auto &[gpu, surface, swapchain] = *static_cast<decltype(glfwUserData)*>(glfwGetWindowUserPointer(window));
            gpu.queues.graphicsPresent.waitIdle();
            swapchain = vulkan::Swapchain { gpu, *surface, extent, swapchain };
        });

        for (std::uint64_t frameIndex = 0; !glfwWindowShouldClose(window); ++frameIndex) {
            gpu.allocator.setCurrentFrameIndex(utils::numeric_cast<std::uint32_t>(frameIndex));
            
            glfwPollEvents();

            ImGui_ImplGlfw_NewFrame();
            ImGui_ImplVulkan_NewFrame();
            ImGui::NewFrame();

            ImGui::DockSpaceOverViewport();
            ImGui::ShowDemoWindow();
            
            ImGui::Render();
            
            // Acquire swapchain image.
            std::uint32_t swapchainImageIndex;
            try {
                swapchainImageIndex = swapchain.acquireNextImage(~0ULL, *swapchainImageAcquireSemaphore).value;
            }
            catch (const vk::OutOfDateKHRError&) {
                continue;
            }
            
            commandPool.reset();
            commandBuffer.begin(vk::CommandBufferBeginInfo {
                vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
            }, *gpu.device.getDispatcher());

            commandBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput,
                {}, {}, {},
                vku::lvalue(vk::ImageMemoryBarrier {
                    {}, vk::AccessFlagBits::eColorAttachmentWrite,
                    {}, vk::ImageLayout::eColorAttachmentOptimal,
                    vk::QueueFamilyIgnored, vk::QueueFamilyIgnored,
                    swapchain.images[swapchainImageIndex], vku::fullSubresourceRange(vk::ImageAspectFlagBits::eColor), 
                }), 
                *gpu.device.getDispatcher());

            commandBuffer.beginRendering({
                {},
                vk::Rect2D { { 0, 0 }, swapchain.extent },
                1,
                {},
                vku::lvalue(vk::RenderingAttachmentInfo {
                    *swapchain.imageViews[swapchainImageIndex], vk::ImageLayout::eColorAttachmentOptimal,
                    {}, {}, {},
                    vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eStore,
                }),
            }, *gpu.device.getDispatcher());
            
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
            
            commandBuffer.endRendering(*gpu.device.getDispatcher());

            commandBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eBottomOfPipe,
                {}, {}, {},
                vku::lvalue(vk::ImageMemoryBarrier {
                    vk::AccessFlagBits::eColorAttachmentWrite, {},
                    vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR,
                    vk::QueueFamilyIgnored, vk::QueueFamilyIgnored,
                    swapchain.images[swapchainImageIndex], vku::fullSubresourceRange(vk::ImageAspectFlagBits::eColor), 
                }), 
                *gpu.device.getDispatcher());
            
            commandBuffer.end(*gpu.device.getDispatcher());
            
            gpu.queues.graphicsPresent.submit(vk::SubmitInfo {
                *swapchainImageAcquireSemaphore,
                vku::lvalue(vk::Flags { vk::PipelineStageFlagBits::eColorAttachmentOutput }),
                commandBuffer,
                *swapchain.imageReadySemaphores[swapchainImageIndex],
            }, *frameReadyFence, *gpu.device.getDispatcher());

            // Present the acquired swapchain image.
            try {
                std::ignore = gpu.queues.graphicsPresent.presentKHR({
                    *swapchain.imageReadySemaphores[swapchainImageIndex],
                    *swapchain,
                    swapchainImageIndex,
                }, *gpu.device.getDispatcher());
            }
            catch (const vk::OutOfDateKHRError&) { }

            std::ignore = gpu.device.waitForFences(*frameReadyFence, true, ~0ULL);
            gpu.device.resetFences(*frameReadyFence);
        }

        gpu.device.waitIdle();

        ImGui_ImplVulkan_Shutdown();
    }

    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}