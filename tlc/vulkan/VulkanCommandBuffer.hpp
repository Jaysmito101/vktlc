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




		friend class VulkanDevice;
	private:
		void Cleanup();

	private:
		VulkanDevice* m_Device = nullptr;

		vk::CommandBuffer m_CommandBuffer = VK_NULL_HANDLE;

		Bool m_IsReady = false;
	};

}