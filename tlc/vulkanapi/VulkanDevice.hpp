#pragma once

#include "vulkanapi/VulkanBase.hpp"

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
		VulkanDevice(VulkanContext* parentContext, vk::PhysicalDevice physicalDevice, const VulkanDeviceSettings& settings, vk::SurfaceKHR);
		~VulkanDevice();

		Ref<VulkanShaderModule> CreateShaderModule(const List<U32>& shaderCode);

		vk::Semaphore CreateVkSemaphore(vk::SemaphoreCreateFlags flags = vk::SemaphoreCreateFlags()) const;
		void DestroyVkSemaphore(vk::Semaphore semaphore) const;

		vk::Fence CreateVkFence(vk::FenceCreateFlags flags = vk::FenceCreateFlags()) const;
		void DestroyVkFence(vk::Fence fence) const;

		vk::DescriptorSetLayout CreateDescriptorSetLayout(const vk::DescriptorSetLayoutCreateInfo& createInfo);
		List<vk::DescriptorSet> AllocateDescriptorSets(const String& group, vk::DescriptorType type, const List<vk::DescriptorSetLayout>& descriptorSetLayout);
		void FreeDescriptorGroup(const String& group);

		U32 FindMemoryType(U32 typeFilter, vk::MemoryPropertyFlags properties) const;

		inline I32 GetGraphicsQueueFamilyIndex() const { return m_QueueFamilyIndices[Graphics]; }
		inline I32 GetComputeQueueFamilyIndex() const { return m_QueueFamilyIndices[Compute]; }
		inline I32 GetTransferQueueFamilyIndex() const { return m_QueueFamilyIndices[Transfer]; }
		inline I32 GetPresentQueueFamilyIndex() const { return m_QueueFamilyIndices[Present]; }

		inline Bool IsReady() const { return m_IsReady; }

		inline void WaitIdle() const { (void)m_Device.waitIdle(); }
		inline Raw<VulkanContext> GetParentContext() const { return m_ParentContext; }
		inline const vk::Device& GetDevice() const { return m_Device; }
		inline const vk::PhysicalDevice& GetPhysicalDevice() const { return m_PhysicalDevice; }
		inline const VulkanDeviceSettings& GetSettings() const { return m_Settings; }
		inline const vk::Queue& GetQueue(VulkanQueueType type) const { return m_Queues[type]; }
		inline const vk::CommandPool& GetCommandPool(VulkanQueueType type) const { return m_CommandPools[type]; }


		friend class VulkanContext;
		friend class VulkanCommandBuffer;
	
	private:
		void CheckExtensionSupport(vk::PhysicalDevice physicalDevice);
		Bool CreateDevice(vk::SurfaceKHR surface = VK_NULL_HANDLE);
		void Cleanup();
		Bool CreateCommandPools();
		vk::DescriptorPool CreateDescriptorPool(vk::DescriptorType type);
		Bool ExpandDescriptorPool(const String& group, vk::DescriptorType type);
		List<vk::DescriptorPool>& GrabDescriptorPools(const String& group, vk::DescriptorType type);

		
		I32 FindAndAddQueueCreateInfo(Bool enable, const vk::QueueFlags& flags, F32* queuePriority, List<vk::DeviceQueueCreateInfo>& queueCreateInfos, vk::SurfaceKHR surface = VK_NULL_HANDLE);
		static I32 FindQueueFamily(const vk::PhysicalDevice& physicalDevice, const vk::QueueFlags& flags, const vk::SurfaceKHR& surface = VK_NULL_HANDLE);
		static vk::DeviceQueueCreateInfo CreateQueueCreateInfo(I32 queueFamilyIndex, F32* queuePriority);

		static Size CalculateDescriptorLayoutCreateInfoHash(const vk::DescriptorSetLayoutCreateInfo& createInfo);

	private:
		Raw<VulkanContext> m_ParentContext;
		VulkanDeviceSettings m_Settings;
		vk::PhysicalDevice m_PhysicalDevice;
		vk::Device m_Device;

		Bool m_SwapchanSupported = false;
		Bool m_MeshShadingSupported = false;
		Bool m_RayTracingSupported = false;

		Array<I32, VulkanQueueType::Count> m_QueueFamilyIndices;

		Array<vk::Queue, VulkanQueueType::Count> m_Queues;

		Array<vk::CommandPool, VulkanQueueType::Count> m_CommandPools;

		Bool m_IsReady = false;
		Set<I32> m_UniqeQueueFamiliesIndices;

		UnorderedMap<vk::DescriptorType, List<vk::DescriptorPool>> m_AvailableDescriptorPools;
		UnorderedMap<String, UnorderedMap<vk::DescriptorType, List<vk::DescriptorPool>>> m_DescriptorPools;
		UnorderedMap<Size, vk::DescriptorSetLayout> m_DescriptorSetLayoutCache;

		UnorderedMap<String, vk::QueryPool> m_QueryPools;
	};
}