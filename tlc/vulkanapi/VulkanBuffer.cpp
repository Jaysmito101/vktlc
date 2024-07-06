#include "vulkanapi/VulkanBuffer.hpp"
#include "vulkanapi/VulkanDevice.hpp"


namespace tlc
{


	VulkanBuffer::VulkanBuffer(VulkanDevice* device)
	{
		m_Device = device;
		m_Size = 0;
	}

	VulkanBuffer::~VulkanBuffer()
	{
		Cleanup();
	}

	void VulkanBuffer::Resize(Size size)
	{
		if (m_Size == size)
			return;

		m_Size = size;
		RecreateBuffer();
	}

	Bool VulkanBuffer::SetData(const void* data, Size size, Size offset)
	{
		if (size + offset > m_Size)
		{
			log::Error("VulkanBuffer::SetData: size + offset > m_Size");
			return false;
		}

		void* mappedData = nullptr;
		if (!MapMemory(&mappedData, size, offset))
		{
			log::Error("VulkanBuffer::SetData: failed to map memory");
			return false;
		}

		memcpy(mappedData, data, size);

		UnmapMemory();

		return true;
	}

	Bool VulkanBuffer::GetData(void* data, Size size, Size offset)
	{
		if (size + offset > m_Size)
		{
			log::Error("VulkanBuffer::GetData: size + offset > m_Size");
			return false;
		}

		void* mappedData = nullptr;
		if (!MapMemory(&mappedData, size, offset))
		{
			log::Error("VulkanBuffer::GetData: failed to map memory");
			return false;
		}

		memcpy(data, mappedData, size);

		UnmapMemory();

		return true;
	}

	Bool VulkanBuffer::MapMemory(void** data, Size size, Size offset)
	{
		if (size + offset > m_Size)
		{
			log::Error("VulkanBuffer::MapMemory: size + offset > m_Size");
			return false;
		}

		auto result = m_Device->GetDevice().mapMemory(m_Memory, offset, size, vk::MemoryMapFlags(), data);

		if (result != vk::Result::eSuccess)
		{
			log::Error("VulkanBuffer::MapMemory: failed to map memory");
			return false;
		}

		return true;
	}

	void VulkanBuffer::UnmapMemory()
	{
		m_Device->GetDevice().unmapMemory(m_Memory);
	}


	Bool VulkanBuffer::RecreateBuffer()
	{
		Cleanup();

		if (m_Size == 0)
			return true;

		auto bufferInfo = vk::BufferCreateInfo()
			.setSize(m_Size)
			.setUsage(m_UsageFlags)
			.setSharingMode(vk::SharingMode::eExclusive);

		auto result = m_Device->GetDevice().createBuffer(&bufferInfo, nullptr, &m_Buffer);

		if (result != vk::Result::eSuccess)
		{
			log::Error("VulkanBuffer::RecreateBuffer: failed to create buffer");
			return false;
		}

		auto memRequirements = m_Device->GetDevice().getBufferMemoryRequirements(m_Buffer);

		auto allocInfo = vk::MemoryAllocateInfo()
			.setAllocationSize(memRequirements.size)
			.setMemoryTypeIndex(m_Device->FindMemoryType(memRequirements.memoryTypeBits, m_MemoryPropertyFlags));

		result = m_Device->GetDevice().allocateMemory(&allocInfo, nullptr, &m_Memory);

		if (result != vk::Result::eSuccess)
		{
			log::Error("VulkanBuffer::RecreateBuffer: failed to allocate buffer memory");
			return false;
		}

		m_Device->GetDevice().bindBufferMemory(m_Buffer, m_Memory, 0);

		m_IsReady = true;

		return true;
	}

	void VulkanBuffer::Cleanup()
	{
		if (!m_IsReady)
			return;

		m_Device->WaitIdle();
		m_Device->GetDevice().destroyBuffer(m_Buffer);
		m_Device->GetDevice().freeMemory(m_Memory);
		m_IsReady = false;
	}
}