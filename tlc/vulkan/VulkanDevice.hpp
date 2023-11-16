#pragma once

#include "vulkan/VulkanBase.hpp"
#include "vulkan/VulkanFramebuffer.hpp"

namespace tlc
{
	class VulkanSwapchain;
	class VulkanContext;
	class VulkanShaderModule;
	class Window;

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
		Ref<VulkanShaderModule> CreateShaderModule(const List<U8>& shaderCode);
		Ref<VulkanFramebuffer> CreateFramebuffer(const VulkanFramebufferSettings& settings = VulkanFramebufferSettings());

		inline I32 GetGraphicsQueueFamilyIndex() const { return m_GraphicsQueueFamilyIndex; }
		inline I32 GetComputeQueueFamilyIndex() const { return m_ComputeQueueFamilyIndex; }
		inline I32 GetTransferQueueFamilyIndex() const { return m_TransferQueueFamilyIndex; }
		inline I32 GetPresentQueueFamilyIndex() const { return m_PresentQueueFamilyIndex; }

		inline Bool IsReady() const { return m_IsReady; }

		inline void WaitIdle() { m_Device.waitIdle(); }
		inline VulkanContext* GetParentContext() const { return m_ParentContext; }
		inline const vk::Device GetDevice() const { return m_Device; }
		inline const vk::PhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }

		friend class VulkanContext;
	private:

		Bool CreateDevice();

	private:
		I32 FindAndAddQueueCreateInfo(Bool enable, const vk::QueueFlags& flags, F32* queuePriority, List<vk::DeviceQueueCreateInfo>& queueCreateInfos);
		static I32 FindQueueFamily(const vk::PhysicalDevice& physicalDevice, const vk::QueueFlags& flags, const vk::SurfaceKHR& surface = VK_NULL_HANDLE);
		static vk::DeviceQueueCreateInfo CreateQueueCreateInfo(I32 queueFamilyIndex, F32* queuePriority);

	private:
		VulkanContext* m_ParentContext;
		vk::PhysicalDevice m_PhysicalDevice;
		vk::Device m_Device;
		
		I32 m_GraphicsQueueFamilyIndex = -1;
		I32 m_ComputeQueueFamilyIndex = -1;
		I32 m_TransferQueueFamilyIndex = -1;
		I32 m_PresentQueueFamilyIndex = -1;

		vk::Queue m_GraphicsQueue;
		vk::Queue m_ComputeQueue;
		vk::Queue m_TransferQueue;
		vk::Queue m_PresentQueue;

		Bool m_IsReady = false;
		Set<I32> m_UniqeQueueFamiliesIndices;

		VulkanDeviceSettings m_Settings;
	};
}