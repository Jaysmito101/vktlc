#pragma once

#include "vulkan/VulkanBase.hpp"


namespace tlc
{
	
	class VulkanDevice;


	class VulkanCommandBuffer
	{
	public:
		VulkanCommandBuffer(VulkanDevice* device, U8 queueType);
		~VulkanCommandBuffer();

		void Reset();

		void Begin();
		void End();
		
		void Submit(List<vk::Semaphore> waitSemaphores, List<vk::Semaphore> signalSemaphores, vk::Fence fence);

		void BeginRenderPass(vk::RenderPass renderPass, vk::Framebuffer framebuffer, vk::Extent2D extent, List<vk::ClearValue> clearValues);
		void EndRenderPass();

		void BindVertexBuffer(vk::Buffer buffer, U64 offset = 0);
		void BindPipeline(vk::Pipeline pipeline, Bool isGraphics = true);
		void SetViewport(vk::Viewport viewport);
		void SetScissor(vk::Rect2D scissor);
		void Draw(U32 vertexCount, U32 instanceCount, U32 firstVertex = 0, U32 firstInstance = 0);

		friend class VulkanDevice;
	private:
		void Cleanup();

	private:
		VulkanDevice* m_Device = nullptr;
		U8 m_QueueType = 0;

		vk::CommandBuffer m_CommandBuffer = VK_NULL_HANDLE;

		Bool m_IsReady = false;
	};

}