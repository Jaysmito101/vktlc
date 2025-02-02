#pragma once

#include "vulkanapi/VulkanBase.hpp"
#include "vulkanapi/VulkanFramebuffer.hpp"

namespace tlc
{
	class VulkanBuffer;
	class VulkanSwapchain;
	class VulkanContext;
	class VulkanShaderModule;
	class VulkanCommandBuffer;
	class Window;

	enum VulkanQueueType : U8
	{
		Graphics = 0,
		Compute,
		Transfer,
		Present,
		Count
	};;

	struct VulkanDeviceSettings
	{
	public:
#if defined(NDEBUG)
		Bool enableValidationLayers = false;
#else
		Bool enableValidationLayers = true;
#endif
		Bool requireGraphicsQueue = true;
		Bool requireComputeQueue = false;
		Bool requireTransferQueue = false;

		float graphicsQueuePriority = 1.0f;
		float computeQueuePriority = 1.0f;
		float transferQueuePriority = 1.0f;


	public:
		VulkanDeviceSettings() = default;

		inline VulkanDeviceSettings& SetEnableValidationLayers(Bool enable) { enableValidationLayers = enable; return *this; }
		inline VulkanDeviceSettings& SetRequireGraphicsQueue(Bool require) { requireGraphicsQueue = require; return *this; }
		inline VulkanDeviceSettings& SetRequireComputeQueue(Bool require) { requireComputeQueue = require; return *this; }

		inline VulkanDeviceSettings& SetGraphicsQueuePriority(float priority) { graphicsQueuePriority = priority; return *this; }
		inline VulkanDeviceSettings& SetComputeQueuePriority(float priority) { computeQueuePriority = priority; return *this; }
		inline VulkanDeviceSettings& SetTransferQueuePriority(float priority) { transferQueuePriority = priority; return *this; }
	};

	class VulkanDevice 
	{
	public:
		VulkanDevice(VulkanContext* parentContext, vk::PhysicalDevice physicalDevice, const VulkanDeviceSettings& settings);
		~VulkanDevice();

		Ref<VulkanSwapchain> CreateSwapchain(Window* window);
		Ref<VulkanShaderModule> CreateShaderModule(const List<U32>& shaderCode);
		Ref<VulkanFramebuffer> CreateFramebuffer(const VulkanFramebufferSettings& settings = VulkanFramebufferSettings());
		Ref<VulkanCommandBuffer> CreateCommandBuffer(VulkanQueueType type);
		Ref<VulkanBuffer> CreateBuffer();

		vk::Semaphore CreateVkSemaphore(vk::SemaphoreCreateFlags flags = vk::SemaphoreCreateFlags()) const;
		void DestroyVkSemaphore(vk::Semaphore semaphore) const;

		vk::Fence CreateVkFence(vk::FenceCreateFlags flags = vk::FenceCreateFlags()) const;
		void DestroyVkFence(vk::Fence fence) const;


		U32 FindMemoryType(U32 typeFilter, vk::MemoryPropertyFlags properties) const;

		inline I32 GetGraphicsQueueFamilyIndex() const { return m_QueueFamilyIndices[Graphics]; }
		inline I32 GetComputeQueueFamilyIndex() const { return m_QueueFamilyIndices[Compute]; }
		inline I32 GetTransferQueueFamilyIndex() const { return m_QueueFamilyIndices[Transfer]; }
		inline I32 GetPresentQueueFamilyIndex() const { return m_QueueFamilyIndices[Present]; }

		inline Bool IsReady() const { return m_IsReady; }

		inline void WaitIdle() const { m_Device.waitIdle(); }
		inline VulkanContext* GetParentContext() const { return m_ParentContext; }
		inline const vk::Device GetDevice() const { return m_Device; }
		inline const vk::PhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }
		inline const VulkanDeviceSettings& GetSettings() const { return m_Settings; }
		inline const vk::Queue GetQueue(VulkanQueueType type) const { return m_Queues[type]; }
		inline const vk::CommandPool GetCommandPool(VulkanQueueType type) const { return m_CommandPools[type]; }
		inline const vk::DescriptorPool GetDescriptorPool() const { return m_DescriptorPool; }


		friend class VulkanContext;
		friend class VulkanCommandBuffer;
	
	private:
		Bool CreateDevice();
		Bool CreateCommandPools();
		Bool CreateDescriptorPool(U32 maxSets);

		I32 FindAndAddQueueCreateInfo(Bool enable, const vk::QueueFlags& flags, F32* queuePriority, List<vk::DeviceQueueCreateInfo>& queueCreateInfos);
		static I32 FindQueueFamily(const vk::PhysicalDevice& physicalDevice, const vk::QueueFlags& flags, const vk::SurfaceKHR& surface = VK_NULL_HANDLE);
		static vk::DeviceQueueCreateInfo CreateQueueCreateInfo(I32 queueFamilyIndex, F32* queuePriority);

	private:
		Raw<VulkanContext> m_ParentContext;
		vk::PhysicalDevice m_PhysicalDevice;
		vk::Device m_Device;
		
		Array<I32, VulkanQueueType::Count> m_QueueFamilyIndices;
		Array<vk::Queue, VulkanQueueType::Count> m_Queues;
		Array<vk::CommandPool, VulkanQueueType::Count> m_CommandPools;
		vk::DescriptorPool m_DescriptorPool = VK_NULL_HANDLE;


		Bool m_IsReady = false;
		Set<I32> m_UniqeQueueFamiliesIndices;


		VulkanDeviceSettings m_Settings;
	};
}