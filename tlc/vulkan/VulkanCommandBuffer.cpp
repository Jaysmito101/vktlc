#include "vulkan/VulkanCommandBuffer.hpp"
#include "vulkan/VulkanDevice.hpp"

namespace tlc
{

	

	VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice* device, U8 queueType)
	{

		m_Device = device;
		m_QueueType = queueType;

		auto allocInfo = vk::CommandBufferAllocateInfo()
			.setCommandPool(device->GetCommandPool(static_cast<VulkanQueueType>(queueType)))
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(1);

		auto result = device->GetDevice().allocateCommandBuffers(allocInfo);

		if (result.size() != 1)
		{
			log::Error("VulkanCommandBuffer::VulkanCommandBuffer: failed to allocate command buffer");
			return;
		}

		m_CommandBuffer = result[0];

		m_IsReady = true;
	}

	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
		Cleanup();
	}

	void VulkanCommandBuffer::Reset()
	{
		m_CommandBuffer.reset();
	}

	void VulkanCommandBuffer::Begin()
	{
		auto beginInfo = vk::CommandBufferBeginInfo()
			// .setFlags(0)
			.setPInheritanceInfo(nullptr);

		m_CommandBuffer.begin(beginInfo);
	}

	void VulkanCommandBuffer::End()
	{
		m_CommandBuffer.end();
	}

	void VulkanCommandBuffer::Submit(List<vk::Semaphore> waitSemaphores, List<vk::Semaphore> signalSemaphores, vk::Fence fence)
	{
		auto queue = m_Device->GetQueue(static_cast<VulkanQueueType>(m_QueueType));
		vk::PipelineStageFlags waitDstStageMask[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

		auto submitInfo = vk::SubmitInfo()
			.setWaitSemaphores(waitSemaphores)
			.setPWaitDstStageMask(waitDstStageMask)
			.setCommandBufferCount(1)
			.setPCommandBuffers(&m_CommandBuffer)
			.setSignalSemaphores(signalSemaphores);

		queue.submit(submitInfo, fence);
	}

	void VulkanCommandBuffer::BeginRenderPass(vk::RenderPass renderPass, vk::Framebuffer framebuffer, vk::Extent2D extent, List<vk::ClearValue> clearValues)
	{
		auto renderArea = vk::Rect2D()
			.setExtent(extent)
			.setOffset(vk::Offset2D(0, 0));

		auto renderPassInfo = vk::RenderPassBeginInfo()
			.setRenderPass(renderPass)
			.setFramebuffer(framebuffer)
			.setRenderArea( renderArea )
			.setClearValueCount(static_cast<U32>(clearValues.size()))
			.setPClearValues(clearValues.data());

		m_CommandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
	}

	void VulkanCommandBuffer::EndRenderPass()
	{
		m_CommandBuffer.endRenderPass();
	}

	void VulkanCommandBuffer::BindPipeline(vk::Pipeline pipeline, Bool isGraphics)
	{		
		m_CommandBuffer.bindPipeline(isGraphics ? vk::PipelineBindPoint::eGraphics : vk::PipelineBindPoint::eCompute, pipeline);
	}

	
	void VulkanCommandBuffer::SetViewport(vk::Viewport viewport)
	{
		m_CommandBuffer.setViewport(0, viewport);
	}

	void VulkanCommandBuffer::SetScissor(vk::Rect2D scissor)
	{
		m_CommandBuffer.setScissor(0, scissor);
	}

	void VulkanCommandBuffer::Draw(U32 vertexCount, U32 instanceCount, U32 firstVertex, U32 firstInstance)
	{
		m_CommandBuffer.draw(vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void VulkanCommandBuffer::Cleanup()
	{
		if (!m_IsReady) 
		{
			return;
		}
		m_Device->WaitIdle();
		m_Device->GetDevice().freeCommandBuffers(m_Device->GetCommandPool(static_cast<VulkanQueueType>(0)), m_CommandBuffer);
		m_IsReady = false;
	}

}