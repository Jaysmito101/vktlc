#include "vulkan/VulkanDevice.hpp"
#include "vulkan/VulkanContext.hpp"
#include "vulkan/VulkanShader.hpp"
#include "vulkan/VulkanSwapchain.hpp"

namespace tlc
{
	VulkanDevice::VulkanDevice(VulkanContext* parentContext, vk::PhysicalDevice physicalDevice, const VulkanDeviceSettings& settings)
		: m_ParentContext(parentContext)
		, m_PhysicalDevice(physicalDevice)
		, m_Settings(settings)
	{
		log::Debug("Creating Vulkan Logical Device");
		if (!CreateDevice())
		{
			log::Error("Failed to create device");
			return;
		}

		log::Info("Vulkan Logical Device created");

		m_IsReady = true;
	}

	VulkanDevice::~VulkanDevice()
	{
		for (auto& swapchain : m_Swapchains)
		{
			swapchain.reset();
		}
		m_Swapchains.clear();

		m_Device.destroy();
	}

	VulkanSwapchain* VulkanDevice::CreateSwapchain(Window* window)
	{
		if (!m_IsReady) 
		{
			log::Error("Device is not ready");
			return nullptr;
		}
		m_Swapchains.push_back(CreateScope<VulkanSwapchain>(this, window));
		return m_Swapchains.back().get();
	}

	Scope<VulkanShaderModule> VulkanDevice::CreateShaderModule(const List<U8>& shaderCode)
	{
		if (!m_IsReady)
		{
			log::Error("Device is not ready");
			return nullptr;
		}
		return CreateScope<VulkanShaderModule>(this, shaderCode);
	}

	Bool VulkanDevice::CreateDevice()
	{

		vk::DeviceCreateInfo deviceCreateInfo = vk::DeviceCreateInfo();

		List<vk::DeviceQueueCreateInfo> queueCreateInfos;

		if ((m_PresentQueueFamilyIndex = m_GraphicsQueueFamilyIndex = FindAndAddQueueCreateInfo(m_Settings.requireGraphicsQueue, vk::QueueFlagBits::eGraphics, &m_Settings.graphicsQueuePriority, queueCreateInfos)) == -1)
		{
			log::Warn("Failed to find graphics queue family");
		}

		if ((m_ComputeQueueFamilyIndex = FindAndAddQueueCreateInfo(m_Settings.requireComputeQueue, vk::QueueFlagBits::eCompute, &m_Settings.computeQueuePriority, queueCreateInfos)) == -1)
		{
			log::Warn("Failed to find compute queue family");
		}

		if ((m_TransferQueueFamilyIndex = FindAndAddQueueCreateInfo(m_Settings.requireTransferQueue, vk::QueueFlagBits::eTransfer, &m_Settings.transferQueuePriority, queueCreateInfos)) == -1)
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

		m_Device = m_PhysicalDevice.createDevice(deviceCreateInfo);

		if (m_Device == static_cast<vk::Device>(VK_NULL_HANDLE))
		{
			log::Error("Failed to create logical device");
			return false;
		}

		if (m_GraphicsQueueFamilyIndex != -1 && m_GraphicsQueueFamilyIndex != 1000) m_GraphicsQueue = m_Device.getQueue(m_GraphicsQueueFamilyIndex, 0);
		if (m_PresentQueueFamilyIndex != -1 && m_PresentQueueFamilyIndex != 1000) m_PresentQueue = m_Device.getQueue(m_PresentQueueFamilyIndex, 0);
		if (m_ComputeQueueFamilyIndex != -1 && m_ComputeQueueFamilyIndex != 1000) m_ComputeQueue = m_Device.getQueue(m_ComputeQueueFamilyIndex, 0);
		if (m_TransferQueueFamilyIndex != -1 && m_TransferQueueFamilyIndex != 1000) m_TransferQueue = m_Device.getQueue(m_TransferQueueFamilyIndex, 0);


		return true;
	}

	I32 VulkanDevice::FindAndAddQueueCreateInfo(Bool enable, const vk::QueueFlags& flags, F32* queuePriority, List<vk::DeviceQueueCreateInfo>& queueCreateInfos)
	{
		if (!enable) return 1000;

		I32 queueFamilyIndex = FindQueueFamily(m_PhysicalDevice, flags, flags & vk::QueueFlagBits::eGraphics ? m_ParentContext->GetSurface() : nullptr);
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
					if (physicalDevice.getSurfaceSupportKHR(i, surface))
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