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

        // create an window resize event hook to handle resizing
        EventManager<EventType::WindowFramebufferSize, I32, I32>::Get()->Subscribe([this](I32 width, I32 height) -> void {
            HandleWindowResize(width, height);
        });
    }

    void VulkanManager::OnEnd()
    {
        m_VulkanSwapchain.reset();
        VulkanContext::Shutdown();
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

        m_PhysicalDevice = m_VulkanContext->PickPhysicalDevice();
        m_VulkanDevice = m_VulkanContext->CreateDevice(m_PhysicalDevice);

        log::Trace("Creating surface");
        m_VulkanContext->CreateSurface(Window::Get());

        log::Trace("Creating swapchain");
        m_VulkanSwapchain = m_VulkanDevice->CreateSwapchain(Window::Get());
        if (m_VulkanSwapchain == nullptr)
        {
            log::Fatal("Failed to create swapchain");
        }

        log::Info("Vulkan setup complete");
    }

    void VulkanManager::HandleWindowResize(U32 width, U32 height)
    {
        // window is minimized
        if (width == 0 || height == 0)
        {
            return;
        }

        // window is already at the same size
        if (width == m_LastWindowSize.first && height == m_LastWindowSize.second)
        {
            return;
        }

        m_LastWindowSize = MakePair(width, height);
        m_VulkanDevice->WaitIdle();
        m_VulkanSwapchain->Recreate();
    }
}