#pragma once

#include "vulkanapi/VulkanBase.hpp"

namespace tlc
{
	class VulkanContext;
	class VulkanDevice;

	struct VulkanBufferSettings {
		Size size = 0;
		vk::BufferUsageFlags usage = vk::BufferUsageFlagBits::eTransferDst;
		vk::MemoryPropertyFlags memoryPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
		vk::SharingMode sharingMode = vk::SharingMode::eExclusive;
		// Raw<VulkanDeviceMemoryPool> deviceMemoryPool = nullptr; // TODO
        Bool useDeviceMemoryPool = false;

		inline VulkanBufferSettings& SetSize(Size s) { size = s; return *this; }
		inline VulkanBufferSettings& SetUsage(vk::BufferUsageFlags u) { usage = u; return *this; }
		inline VulkanBufferSettings& SetMemoryPropertyFlags(vk::MemoryPropertyFlags flags) { memoryPropertyFlags = flags; return *this; }
		inline VulkanBufferSettings& SetSharingMode(vk::SharingMode mode) { sharingMode = mode; return *this; }
		inline VulkanBufferSettings& SetUseDeviceMemoryPool(Bool u) { useDeviceMemoryPool = u; return *this; }
		// inline VulkanBufferSettings& SetDeviceMemoryPool(Raw<VulkanDeviceMemoryPool> p) { deviceMemoryPool = p; return *this; }
	};

	class VulkanBuffer
	{
	public:
		VulkanBuffer(Raw<VulkanDevice> device, const VulkanBufferSettings& settings = VulkanBufferSettings());
		~VulkanBuffer();

		Bool SetData(const void* data, Size size, Size offset = 0);
		Bool GetData(void* data, Size size, Size offset = 0);
		Bool MapMemory(Size size, Size offset, void** data = nullptr);
		void UnmapMemory();
		Bool Flush(Size size, Size offset = 0);
		Bool Invalidate(Size size, Size offset = 0);

		inline vk::Buffer GetBuffer() const { return m_Buffer; }
		inline bool IsReady() const { return m_IsReady; }
		inline Size GetSize() const { return m_Size; }
		inline vk::BufferUsageFlags GetUsageFlags() const { return m_UsageFlags; }
		inline Size GetMappedSize() const { return m_MappedSize; }
		inline Size GetMappedOffset() const { return m_MappedOffset; }

		inline VulkanBufferSettings& GetSettings() { return m_Settings; }

		inline void* GetMappedDataRaw() const { return m_MappedData; }

		template<typename T>
		inline Raw<T> GetMappedData() const { return static_cast<T*>(m_MappedData); }


		inline static Ref<VulkanBuffer> CreateStagingBuffer(Raw<VulkanDevice> device, Size size) {
			auto bufferSettings = VulkanBufferSettings()
				.SetSize(size)
				.SetUsage(vk::BufferUsageFlagBits::eTransferSrc)
				.SetMemoryPropertyFlags(vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
			auto buffer = CreateRef<VulkanBuffer>(device, bufferSettings);
			if (!buffer->IsReady()) {
				log::Error("Failed to create staging buffer");
				return nullptr;
			}
			return buffer;				
		}

	private:
		Bool Recreate();
		void Cleanup();


	private:
		Raw<VulkanDevice> m_Device = nullptr;

		VulkanBufferSettings m_Settings;

		Size m_Size = 0;
		vk::BufferUsageFlags m_UsageFlags = vk::BufferUsageFlagBits::eTransferDst;

		void* m_MappedData = nullptr;
		vk::DeviceSize m_MappedSize = 0;
		vk::DeviceSize m_MappedOffset = 0;

		vk::Buffer m_Buffer = VK_NULL_HANDLE;
		vk::DeviceMemory m_Memory = VK_NULL_HANDLE;
		Bool m_IsReady = false;
	};


}