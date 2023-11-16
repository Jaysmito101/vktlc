#include "vulkan/VulkanCommandBuffer.hpp"
#include "vulkan/VulkanDevice.hpp"

namespace tlc
{

	

	VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice* device, U8 queueType)
	{
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

		m_IsReady = true;
	}

	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
		Cleanup();
	}

	void VulkanCommandBuffer::Cleanup()
	{
		if (!m_IsReady) 
		{
			return;
		}

		m_Device->GetDevice().freeCommandBuffers(m_Device->GetCommandPool(static_cast<VulkanQueueType>(0)), m_CommandBuffer);

		m_IsReady = false;
	}

}