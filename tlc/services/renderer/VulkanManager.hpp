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

    private:
        void SetupVulkan();
        void ShutdownVulkan();

        void HandleWindowResize(U32 width, U32 height);

        vk::PhysicalDevice m_PhysicalDevice;
        Raw<VulkanDevice> m_VulkanDevice = nullptr;
        Raw<VulkanContext> m_VulkanContext = nullptr;
        Raw<VulkanSwapchain> m_VulkanSwapchain = nullptr;

        Pair<I32, I32> m_LastWindowSize = MakePair(0, 0);
    };

}