#pragma once 

#include "core/Core.hpp"
#include "services/Services.hpp"
#include "vulkanapi/VulkanContext.hpp"
#include "vulkanapi/VulkanDevice.hpp"
#include "vulkanapi/VulkanSwapchain.hpp"

namespace tlc {

    class VulkanManager : public IService {
    public:
        void Setup();
        virtual void OnStart() override;
        virtual void OnEnd() override;
        virtual void OnSceneChange() override;
        virtual void OnEvent(const String& event, const String& eventParams) override;

        Raw<VulkanSwapchain> GetSwapchain() { return m_VulkanSwapchain; }
        Raw<VulkanDevice> GetDevice() { return m_VulkanDevice; }
        Raw<VulkanContext> GetContext() { return m_VulkanContext; }
        const vk::PhysicalDevice& GetPhysicalDevice() { return m_PhysicalDevice; }


    private:
        void SetupVulkan();
        void ShutdownVulkan();

        vk::PhysicalDevice m_PhysicalDevice;
        Raw<VulkanDevice> m_VulkanDevice = nullptr;
        Raw<VulkanContext> m_VulkanContext = nullptr;
        Raw<VulkanSwapchain> m_VulkanSwapchain = nullptr;
    };

}