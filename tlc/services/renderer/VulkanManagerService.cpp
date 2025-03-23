#include "services/renderer/VulkanManager.hpp"
#include "core/Window.hpp"

namespace tlc
{
    void VulkanManager::Setup()
    {
    }

    void VulkanManager::OnStart()
    {
        // Setup vulkan context, device and swapchain, etc
        SetupVulkan();
    }

    void VulkanManager::OnEnd()
    {
        log::Trace("Vulkan shutdown");
    }

    void VulkanManager::OnSceneChange()
    {
    }

    void VulkanManager::OnEvent(const String& event, const String& eventParams)
    {
    }

    void VulkanManager::SetupVulkan()
    {
        log::Trace("Setting up Vulkan");

        m_VulkanContext = VulkanContext::Get();

        log::Trace("Creating surface");
        auto surfaceKHR =  m_VulkanContext->CreateSurface(Window::Get());

        m_PhysicalDevice = m_VulkanContext->PickPhysicalDevice();
        m_VulkanDevice = m_VulkanContext->CreateDevice(m_PhysicalDevice, surfaceKHR);
        log::Trace("Creating swapchain");
        m_VulkanSwapchain = m_VulkanContext->CreateSwapchain(Window::Get(), m_VulkanDevice, surfaceKHR);
        if (m_VulkanSwapchain == nullptr)
        {
            log::Fatal("Failed to create swapchain");
        }

        log::Info("Vulkan setup complete");
    }
}