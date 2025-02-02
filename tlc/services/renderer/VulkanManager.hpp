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

        inline Raw<VulkanDevice> GetDevice() { return m_VulkanDevice; }
        inline Raw<VulkanContext> GetContext() { return m_VulkanContext; }
        inline const vk::PhysicalDevice& GetPhysicalDevice() { return m_PhysicalDevice; }

    private:
        void SetupVulkan();
        void ShutdownVulkan();

        void HandleWindowResize(U32 width, U32 height);

        Raw<VulkanDevice> m_VulkanDevice = nullptr;
        Raw<VulkanContext> m_VulkanContext = nullptr;
        Ref<VulkanSwapchain> m_VulkanSwapchain = nullptr;

        vk::PhysicalDevice m_PhysicalDevice;   

        Pair<I32, I32> m_LastWindowSize = MakePair(0, 0);
    };

}