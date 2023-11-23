#pragma once

#include "vulkan/VulkanContext.hpp"

namespace tlc
{


	class Renderer
	{
	public:
		Renderer(VulkanDevice* device, VulkanSwapchain* swapchain);
		~Renderer();

		void AcquireNextImage();
		void BeginFrame();
		void EndFrame();
		void PresentFrame();

		void BeginDefaultRenderPass();
		void EndRenderPass();
		
		void SetPipeline(VulkanGraphicsPipeline* pipeline);

		void SetViewport(F32 x, F32 y, F32 width, F32 height, F32 minDepth = 0.0f, F32 maxDepth = 1.0f);
		void SetScissor(I32 x, I32 y, U32 width, U32 height);
		
		void DrawRaw(U32 vertexCount, U32 instanceCount = 1, U32 firstVertex = 0, U32 firstInstance = 0); // TEMP


		inline Bool IsReady() const { return m_IsReady; }
		inline void SetClearColor(vk::ClearColorValue color) { m_ClearColor = color; }
		inline void SetClearColor(F32 r, F32 g, F32 b, F32 a) { m_ClearColor = { r, g, b, a }; }
		inline VulkanCommandBuffer* GetCommandBuffer() const { return m_CommandBuffer.get(); }

		inline static Renderer* Get(VulkanDevice* device = nullptr, VulkanSwapchain* swapchain = nullptr) { if (s_Instance == nullptr) { s_Instance = CreateScope<Renderer>(device, swapchain); } return s_Instance.get(); }
		inline static void Shutdown() { s_Instance.reset(); }

	private:
		void Cleanup();
		Bool CreateVulkanObjects();

	private:
		VulkanDevice* m_Device;
		VulkanSwapchain* m_Swapchain = nullptr;


		vk::Semaphore m_ImageAvailableSemaphore = VK_NULL_HANDLE;
		vk::Semaphore m_RenderFinishedSemaphore = VK_NULL_HANDLE;
		vk::Fence m_InFlightFence = VK_NULL_HANDLE;
		vk::ClearColorValue m_ClearColor = { 0.2f, 0.2f, 0.2f, 1.0f };
		Ref<VulkanCommandBuffer> m_CommandBuffer = nullptr;


		U32 m_SwapchainImageIndex = 0;

		Bool m_IsReady = false;

		static Scope<Renderer> s_Instance;
	};

}