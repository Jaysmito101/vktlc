#include "vulkanapi/VulkanDevice.hpp"
#include "vulkanapi/VulkanContext.hpp"
#include "vulkanapi/VulkanShader.hpp"
#include "vulkanapi/VulkanSwapchain.hpp"
#include "vulkanapi/VulkanBuffer.hpp"

namespace tlc
{
	VulkanDevice::VulkanDevice(VulkanContext* parentContext, vk::PhysicalDevice physicalDevice, const VulkanDeviceSettings& settings, vk::SurfaceKHR surface)
		: m_ParentContext(parentContext)
		, m_PhysicalDevice(physicalDevice)
		, m_Settings(settings)
	{
		log::Debug("Creating Vulkan Logical Device");
		if (!CreateDevice(surface))
		{
			log::Error("Failed to create device");
			return;
		}

		log::Info("Vulkan Logical Device created");

		m_IsReady = true;
	}

	VulkanDevice::~VulkanDevice()
	{
		(void)m_Device.waitIdle();


		for (auto& commandPool : m_CommandPools)
		{
			if (commandPool == static_cast<vk::CommandPool>(VK_NULL_HANDLE)) continue;
			m_Device.destroyCommandPool(commandPool);
		}

		m_Device.destroy();
	}

	Ref<VulkanShaderModule> VulkanDevice::CreateShaderModule(const List<U32>& shaderCode)
	{
		if (!m_IsReady)
		{
			log::Error("Device is not ready");
			return nullptr;
		}
		return CreateRef<VulkanShaderModule>(this, shaderCode);
	}

	vk::Semaphore VulkanDevice::CreateVkSemaphore(vk::SemaphoreCreateFlags flags) const
	{
		auto semaphoreCreateInfo = vk::SemaphoreCreateInfo()
			.setFlags(flags);

		auto [result, semaphore] = m_Device.createSemaphore(semaphoreCreateInfo);
		VkCall(result);
		return semaphore;
	}

	void VulkanDevice::DestroyVkSemaphore(vk::Semaphore semaphore) const
	{
		WaitIdle();
		m_Device.destroySemaphore(semaphore);
	}

	vk::Fence VulkanDevice::CreateVkFence(vk::FenceCreateFlags flags) const
	{
		auto fenceCreateInfo = vk::FenceCreateInfo()
			.setFlags(flags);

		auto [result, fence] = m_Device.createFence(fenceCreateInfo);
		VkCall(result);
		return fence;
	}

	void VulkanDevice::DestroyVkFence(vk::Fence fence) const
	{
		WaitIdle();
		m_Device.destroyFence(fence);
	}

	U32 VulkanDevice::FindMemoryType(U32 typeFilter, vk::MemoryPropertyFlags properties) const
	{
		auto memoryProperties = m_PhysicalDevice.getMemoryProperties();

		for (U32 i = 0; i < memoryProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		log::Error("Failed to find suitable memory type");
		return std::numeric_limits<U32>::max();
	}

	Bool VulkanDevice::CreateDevice(vk::SurfaceKHR surface)
	{

		vk::DeviceCreateInfo deviceCreateInfo = vk::DeviceCreateInfo();

		List<vk::DeviceQueueCreateInfo> queueCreateInfos;

		if ((m_QueueFamilyIndices[Present] = m_QueueFamilyIndices[Graphics] = FindAndAddQueueCreateInfo(m_Settings.requireGraphicsQueue, vk::QueueFlagBits::eGraphics, &m_Settings.graphicsQueuePriority, queueCreateInfos, surface)) == -1)
		{
			log::Warn("Failed to find graphics queue family");
		}

		if ((m_QueueFamilyIndices[Compute] = FindAndAddQueueCreateInfo(m_Settings.requireComputeQueue, vk::QueueFlagBits::eCompute, &m_Settings.computeQueuePriority, queueCreateInfos)) == -1)
		{
			log::Warn("Failed to find compute queue family");
		}

		if ((m_QueueFamilyIndices[Transfer] = FindAndAddQueueCreateInfo(m_Settings.requireTransferQueue, vk::QueueFlagBits::eTransfer, &m_Settings.transferQueuePriority, queueCreateInfos)) == -1)
		{
			log::Warn("Failed to find transfer queue family");
		}

		deviceCreateInfo.setQueueCreateInfoCount(static_cast<U32>(queueCreateInfos.size()))
			.setPQueueCreateInfos(queueCreateInfos.data())
			.setEnabledLayerCount(0);

		// NOTE: I am not checking for extensions support here as the target devices for this engine support must swapchain extension
		List<CString> extensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		deviceCreateInfo.setEnabledExtensionCount(static_cast<U32>(extensions.size()))
			.setPpEnabledExtensionNames(extensions.data());

		if (m_Settings.enableValidationLayers)
		{
			const auto& layers = m_ParentContext->GetLayers();
			deviceCreateInfo.setEnabledLayerCount(static_cast<U32>(layers.size()))
				.setPpEnabledLayerNames(layers.data());
		}

		auto [result, device] = m_PhysicalDevice.createDevice(deviceCreateInfo);
		VkCritCall(result);
		m_Device = device;

		if (m_Device == static_cast<vk::Device>(VK_NULL_HANDLE))
		{
			log::Error("Failed to create logical device");
			return false;
		}

		for (U32 i = 0; i < VulkanQueueType::Count; i++)
		{
			if (m_QueueFamilyIndices[i] == -1 || m_QueueFamilyIndices[i] == 1000) continue;
			m_Queues[i] = m_Device.getQueue(m_QueueFamilyIndices[i], 0);
			if (m_Queues[i] == static_cast<vk::Queue>(VK_NULL_HANDLE))
			{
				log::Error("Failed to get queue");
				return false;
			}
		}

		if (!CreateCommandPools())
		{
			log::Error("Failed to create command pools");
			return false;
		}

		return true;
	}

	Bool VulkanDevice::CreateCommandPools()
	{
		auto poolCreateInfo = vk::CommandPoolCreateInfo()
			.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);


		for (U32 i = 0; i < VulkanQueueType::Count; i++)
		{
			if (m_QueueFamilyIndices[i] == -1 || m_QueueFamilyIndices[i] == 1000)
			{
				m_CommandPools[i] = VK_NULL_HANDLE;
				continue;
			}
			poolCreateInfo.setQueueFamilyIndex(m_QueueFamilyIndices[i]);
			
			auto [result, commandPool] = m_Device.createCommandPool(poolCreateInfo);
			VkCritCall(result);
			m_CommandPools[i] = commandPool;

			if (m_CommandPools[i] == static_cast<vk::CommandPool>(VK_NULL_HANDLE))
			{
				log::Error("Failed to create command pool");
				return false;
			}
		}

		return true;
	}

	I32 VulkanDevice::FindAndAddQueueCreateInfo(Bool enable, const vk::QueueFlags& flags, F32* queuePriority, List<vk::DeviceQueueCreateInfo>& queueCreateInfos, vk::SurfaceKHR surface)
	{
		if (!enable) return 1000;

		I32 queueFamilyIndex = FindQueueFamily(m_PhysicalDevice, flags, flags & vk::QueueFlagBits::eGraphics ? surface : nullptr);
		if (queueFamilyIndex == -1)
		{
			log::Warn("Failed to find queue family");
			return queueFamilyIndex;
		}

		if (m_UniqeQueueFamiliesIndices.find(queueFamilyIndex) != m_UniqeQueueFamiliesIndices.end())
		{
			log::Warn("Queue family already exists");
			return queueFamilyIndex;
		};
		m_UniqeQueueFamiliesIndices.insert(queueFamilyIndex);

		queueCreateInfos.push_back(CreateQueueCreateInfo(queueFamilyIndex, queuePriority));
		return queueFamilyIndex;
	}

	I32 VulkanDevice::FindQueueFamily(const vk::PhysicalDevice& physicalDevice, const vk::QueueFlags& flags, const vk::SurfaceKHR& surface)
	{
		auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

		for (I32 i = 0; i < queueFamilyProperties.size(); ++i)
		{
			if (queueFamilyProperties[i].queueFlags & flags)
			{
				if (surface)
				{
					if (physicalDevice.getSurfaceSupportKHR(i, surface).value)
					{
						return i;
					}
				}
				else
				{
					return i;
				}
			}
		}

		return -1;
	}

	vk::DeviceQueueCreateInfo VulkanDevice::CreateQueueCreateInfo(I32 queueFamilyIndex, F32* queuePriority)
	{
		vk::DeviceQueueCreateInfo queueCreateInfo = vk::DeviceQueueCreateInfo()
			.setQueueFamilyIndex(queueFamilyIndex)
			.setQueueCount(1)
			.setPQueuePriorities(queuePriority);
		return queueCreateInfo;
	}
}