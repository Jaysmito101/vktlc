#include "vulkanapi/VulkanBuffer.hpp"
#include "vulkanapi/VulkanDevice.hpp"

namespace tlc
{

	VulkanBuffer::VulkanBuffer(Raw<VulkanDevice> device, const VulkanBufferSettings &settings)
		: m_Device(device), m_Settings(settings)
	{
		if (!Recreate())
		{
			log::Warn("Failed to create Vulkan buffer");
		}
	}

	VulkanBuffer::~VulkanBuffer()
	{
		if (m_IsReady)
		{
			Cleanup();
		}
	}

	Bool VulkanBuffer::SetData(const void *data, Size size, Size offset)
	{
		if (size + offset > m_Size)
		{
			log::Error("VulkanBuffer::SetData: size + offset > m_Size");
			return false;
		}

		if (!MapMemory(size, offset))
		{
			log::Error("VulkanBuffer::SetData: failed to map memory");
			return false;
		}
		memcpy(m_MappedData, data, size);
		UnmapMemory();

		return true;
	}

	Bool VulkanBuffer::GetData(void *data, Size size, Size offset)
	{
		if (size + offset > m_Size)
		{
			log::Error("VulkanBuffer::GetData: size + offset > m_Size");
			return false;
		}

		if (!MapMemory(size, offset))
		{
			log::Error("VulkanBuffer::GetData: failed to map memory");
			return false;
		}
		memcpy(data, m_MappedData, size);
		UnmapMemory();
		return true;
	}

	Bool VulkanBuffer::MapMemory(Size size, Size offset, void **data)
	{
		if (size + offset > m_Size)
		{
			log::Error("VulkanBuffer::MapMemory: size + offset > m_Size");
			return false;
		}
		auto result = m_Device->GetDevice().mapMemory(m_Memory, offset, size, vk::MemoryMapFlags(), &m_MappedData);
		if (result != vk::Result::eSuccess)
		{
			VkCall(result);
			return false;
		}
		if (data != nullptr)
		{
			*data = m_MappedData;
		}
		m_MappedSize = size;
		m_MappedOffset = offset;
		return true;
	}

	void VulkanBuffer::UnmapMemory()
	{
		if (m_MappedData == nullptr)
		{
			log::Error("VulkanBuffer::UnmapMemory: memory is not mapped");
			return;
		}
		m_Device->GetDevice().unmapMemory(m_Memory);
		m_MappedData = nullptr;
		m_MappedSize = 0;
		m_MappedOffset = 0;
	}

	Bool VulkanBuffer::Recreate()
	{
		if (m_IsReady)
		{
			log::Trace("VulkanBuffer::Recreate: buffer is already ready, cleaning up first");
			Cleanup();
		}

		if (m_Settings.size == 0)
		{
			log::Error("VulkanBuffer::Recreate: size is 0");
			return false;
		}

		auto bufferInfo = vk::BufferCreateInfo()
							  .setSize(m_Settings.size)
							  .setUsage(m_Settings.usage)
							  .setSharingMode(m_Settings.sharingMode);

		auto [bufferResult, buffer] = m_Device->GetDevice().createBuffer(bufferInfo);
		if (bufferResult != vk::Result::eSuccess)
		{
			VkCall(bufferResult);
			return false;
		}
		m_Buffer = buffer;

		auto memRequirements = m_Device->GetDevice().getBufferMemoryRequirements(m_Buffer);
		if (m_Settings.useDeviceMemoryPool) {
			log::Fatal("Device Memory Pool has not yet been implemented!");
		}
		else {
			auto allocInfo = vk::MemoryAllocateInfo()
								 .setAllocationSize(memRequirements.size)
								 .setMemoryTypeIndex(m_Device->FindMemoryType(memRequirements.memoryTypeBits, m_Settings.memoryPropertyFlags));

			auto [result, memory] = m_Device->GetDevice().allocateMemory(allocInfo);
			if (result != vk::Result::eSuccess)
			{
				VkCall(result);
				return false;
			}
			m_Memory = memory;
		}
		VkCall(m_Device->GetDevice().bindBufferMemory(m_Buffer, m_Memory, 0));

		m_Size = m_Settings.size;
		m_UsageFlags = m_Settings.usage;
		m_IsReady = true;

		return true;
	}

	void VulkanBuffer::Cleanup()
	{
		if (!m_IsReady) {
			log::Trace("VulkanBuffer::Cleanup: buffer is not ready, cannot cleanup");
			return;
		}

		if (m_MappedData != nullptr) {
			UnmapMemory();
		}

		m_Device->WaitIdle();

		m_Device->GetDevice().destroyBuffer(m_Buffer);

		if (m_Settings.useDeviceMemoryPool) {
			log::Fatal("Device Memory Pool has not yet been implemented!");
		}
		else {
			m_Device->GetDevice().freeMemory(m_Memory);
		}

		m_IsReady = false;
	}

	Bool VulkanBuffer::Flush(Size size, Size offset) {
		if (size + offset > m_Size) {
			log::Error("VulkanBuffer::Flush: size + offset > m_Size");
			return false;
		}
		
		if (m_Settings.useDeviceMemoryPool) {
			log::Fatal("Device Memory Pool has not yet been implemented!");
		}
		else {
			auto mappedRange = vk::MappedMemoryRange()
				.setMemory(m_Memory)
				.setSize(size)
				.setOffset(offset);
			auto result = m_Device->GetDevice().flushMappedMemoryRanges(mappedRange);
			if (result != vk::Result::eSuccess) {
				VkCall(result);
				return false;
			}
			return true;
		}
	}
	
	Bool VulkanBuffer::Invalidate(Size size, Size offset) {
		if (size + offset > m_Size) {
			log::Error("VulkanBuffer::Invalidate: size + offset > m_Size");
			return false;
		}
		
		if (m_Settings.useDeviceMemoryPool) {
			log::Fatal("Device Memory Pool has not yet been implemented!");
		}
		else {
			auto mappedRange = vk::MappedMemoryRange()
				.setMemory(m_Memory)
				.setSize(size)
				.setOffset(offset);
			auto result = m_Device->GetDevice().invalidateMappedMemoryRanges(mappedRange);
			if (result != vk::Result::eSuccess) {
				VkCall(result);
				return false;
			}
			return true;
		}
	}

	Bool VulkanBuffer::ResizeToAtleast(Size size) {
		if (size <= m_Size) {
			return true; // No resize needed
		}
		
		m_Settings.size = size;
		Cleanup();
		return Recreate();
	}

    VulkanBufferAsyncUploadResult VulkanBuffer::UploadAsync(
        const VulkanBufferUploadSettings& uploadSettings,
        vk::CommandBuffer commandBuffer
    ) {
        // If the buffer is host visible, we can directly map and copy
        if (m_Settings.memoryPropertyFlags & vk::MemoryPropertyFlagBits::eHostVisible) {
            if (!SetData(uploadSettings.data, uploadSettings.size, uploadSettings.offset)) {
                return VulkanBufferAsyncUploadResult::Faliure("Failed to set data for host visible buffer");
            }
            return VulkanBufferAsyncUploadResult { .success = true };
        }

        // For device local buffers, we need to use a staging buffer
        Ref<VulkanBuffer> stagingBuffer = nullptr;
        if (uploadSettings.stagingBuffer && uploadSettings.stagingBuffer->IsReady() && uploadSettings.stagingBuffer->GetSize() >= uploadSettings.size) {
            stagingBuffer = uploadSettings.stagingBuffer;
        } else {
            stagingBuffer = CreateStagingBuffer(m_Device, uploadSettings.size);
        }

        if (!stagingBuffer->IsReady()) {
            return VulkanBufferAsyncUploadResult::Faliure("Failed to create staging buffer for buffer upload");
        }

        if (!stagingBuffer->SetData(uploadSettings.data, uploadSettings.size)) {
            return VulkanBufferAsyncUploadResult::Faliure("Failed to set data for staging buffer");
        }

        auto bufferCopyRegion = vk::BufferCopy()
            .setSrcOffset(0)
            .setDstOffset(uploadSettings.offset)
            .setSize(uploadSettings.size);

        commandBuffer.copyBuffer(
            stagingBuffer->GetBuffer(),
            m_Buffer,
            { bufferCopyRegion }
        );

        auto result = VulkanBufferAsyncUploadResult {
            .success = true,
            .stagingBuffer = stagingBuffer
        };
        
        return result;
    }

    Bool VulkanBuffer::UploadSync(const VulkanBufferUploadSettings& uploadSettings) {
        auto commandBufferAllocateInfo = vk::CommandBufferAllocateInfo()
            .setCommandPool(m_Device->GetCommandPool(uploadSettings.queueType))
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount(1);

        auto [result, commandBuffers] = m_Device->GetDevice().allocateCommandBuffers(commandBufferAllocateInfo);
        if (result != vk::Result::eSuccess) {
            VkCall(result);
            return false;
        }
        auto commandBuffer = commandBuffers[0];

        auto commandBufferBeginInfo = vk::CommandBufferBeginInfo()
            .setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit)
            .setPInheritanceInfo(nullptr);
        VkCall(commandBuffer.begin(commandBufferBeginInfo));

        auto uploadResult = UploadAsync(uploadSettings, commandBuffer);

        VkCall(commandBuffer.end());

        auto submitInfo = vk::SubmitInfo()
            .setCommandBufferCount(1)
            .setPCommandBuffers(&commandBuffer);

        auto fence = m_Device->CreateVkFence(vk::FenceCreateFlagBits::eSignaled);
        m_Device->GetDevice().resetFences({fence});
        VkCall(m_Device->GetQueue(uploadSettings.queueType).submit({ submitInfo }, fence));
        VkCall(m_Device->GetDevice().waitForFences({ fence }, VK_TRUE, UINT64_MAX));

        m_Device->DestroyVkFence(fence);
        m_Device->GetDevice().freeCommandBuffers(m_Device->GetCommandPool(uploadSettings.queueType), { commandBuffer });

        return uploadResult.success;
    }
}