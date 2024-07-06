#pragma once

#include "vulkanapi/VulkanBase.hpp"

namespace tlc
{

	class VulkanContext;
	class VulkanDevice;

	class VulkanBuffer
	{
	public:
		VulkanBuffer(VulkanDevice* device);
		~VulkanBuffer();

		void Resize(Size size);


		Bool SetData(const void* data, Size size, Size offset = 0);
		Bool GetData(void* data, Size size, Size offset = 0);
		Bool MapMemory(void** data, Size size, Size offset = 0);
		void UnmapMemory();

		inline vk::Buffer GetBuffer() const { return m_Buffer; }
		inline vk::DeviceMemory GetMemory() const { return m_Memory; }
		inline bool IsReady() const { return m_IsReady; }
		inline Size GetSize() const { return m_Size; }

		inline void SetUsageFlags(vk::BufferUsageFlags usageFlags) { m_UsageFlags = usageFlags; }


	private:
		Bool RecreateBuffer();
		void Cleanup();


	private:
		Raw<VulkanDevice> m_Device = nullptr;
		vk::BufferUsageFlags m_UsageFlags = vk::BufferUsageFlagBits::eVertexBuffer;
		vk::MemoryPropertyFlags m_MemoryPropertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
		vk::Buffer m_Buffer = VK_NULL_HANDLE;
		vk::DeviceMemory m_Memory = VK_NULL_HANDLE;
		Size m_Size = 0;
		Bool m_IsReady = false;
	};


}