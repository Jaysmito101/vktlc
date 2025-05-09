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

        Bool RenderCurrentFrame(F32 deltaTime);
        
        inline Size GetCurrentFrameIndex() const { return m_CurrentFrameIndex; }
        inline Size GetNumInflightFrames() const { return m_NumInflightFrames; }
        inline vk::RenderPass GetRenderPass() const { return m_RenderPass; }

    private:
		void CreateRenderPass();
		void DestroyRenderPass();

		void CreateSynchronizationObjects();
		void DestroySynchronizationObjects();

        void CreateFramebuffers();
        void DestroyFramebuffers();

        void CreateCommandBuffers();
        void DestroyCommandBuffers();

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
        List<vk::Framebuffer> m_Framebuffers;
        List<vk::CommandBuffer> m_CommandBuffers;

        Pair<I32, I32> m_LastWindowSize = MakePair(0, 0);
        vk::CommandBuffer m_CurrentCommandBuffer = VK_NULL_HANDLE;
		vk::RenderPass m_RenderPass = VK_NULL_HANDLE;
    };

}