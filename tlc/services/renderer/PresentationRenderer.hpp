#pragma once 

#include "core/Core.hpp"
#include "services/Services.hpp"
#include "services/renderer/VulkanManager.hpp"
#include "vulkanapi/VulkanContext.hpp"
#include "vulkanapi/VulkanDevice.hpp"
#include "vulkanapi/VulkanSwapchain.hpp"

namespace tlc {

    class PresentationRenderer : public IService {
    public:
        void Setup();
        virtual void OnStart() override;
        virtual void OnEnd() override;
        virtual void OnSceneChange() override;
        virtual void OnEvent(const String& event, const String& eventParams) override;

        void BeginRenderingCurrentFrame();
        void EndRenderingCurrentFrame();

        inline Size GetCurrentFrameIndex() const { return m_CurrentFrameIndex; }
        inline Size GetNumInflightFrames() const { return m_NumInflightFrames; }

    private:
		void CreateRenderPass();
		void DestroyRenderPass();

		void CreateSynchronizationObjects();
		void DestroySynchronizationObjects();

        void CreatePipeline();

		void RecreateRenderResources();


    private:
        U32 m_CurrentImageIndex = 0;

        Scope<VulkanGraphicsPipeline> m_Pipeline = nullptr;

        Size m_NumInflightFrames = 0;
		Size m_CurrentFrameIndex = 0;
		List<vk::Semaphore> m_ImageAvailableSemaphores;
		List<vk::Semaphore> m_RenderFinishedSemaphores;
		List<vk::Fence> m_InFlightFences;
		vk::RenderPass m_RenderPass = VK_NULL_HANDLE;
    };

}