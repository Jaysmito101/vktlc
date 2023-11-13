#include "vulkan/VulkanContext.hpp"

#include <GLFW/glfw3.h>

namespace tlc
{

	static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) 
	{
		(void)messageType; (void)pUserData;

		LogLevel level = LogLevel::Trace;
		switch (messageSeverity)
		{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:			level = LogLevel::Trace; break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:				level = LogLevel::Info; break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:			level = LogLevel::Warning; break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:				level = LogLevel::Error; break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:	level = LogLevel::Fatal; break;
		default:														level = LogLevel::Trace;
		}

		log::Log(level, "Validation Layer: {}", pCallbackData->pMessage);

		return VK_FALSE;
	}
	 
	Scope<VulkanContext> VulkanContext::s_Instance = nullptr;

	VulkanContext::VulkanContext()
	{
		if (s_Instance != nullptr)
		{
			log::Fatal("VulkanContext already exists!");
		}

		log::Debug("Creating VulkanContext");


		// print available extensions
		const auto extensions = QueryAvailableExtensions();
		log::Trace("Available Vulkan extensions:");
		for (const auto& extension : extensions)
		{
			log::Trace("\t{}", extension);
		}

		// print available layers
		const auto layers = QueryAvailableLayers();
		log::Trace("Available Vulkan layers:");
		for (const auto& layer : layers)
		{
			log::Trace("\t{}", layer);
		}

		if (!CreateInstance())
		{
			log::Fatal("Failed to create Vulkan instance");
		}

		const auto physicalDevice = QueryPhysicalDevices();
		log::Trace("Available Vulkan devices:");
		for (const auto& device : physicalDevice)
		{
			log::Trace("\tDevice: {}", device.getProperties().deviceName.data());
		}
		
		log::Info("VulkanContext created");
	}

	VulkanContext::~VulkanContext()
	{
		log::Debug("Shutting down VulkanContext");
#if !defined(NDEBUG)
		DestroyDebugMessenger();
#endif
		m_Instance.destroy();
		log::Info("VulkanContext shutdown");
	}

	List<String> VulkanContext::QueryAvailableExtensions()
	{
		List<String> extensions;

		U32 extensionCount = 0;
		VkCall(vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr));

		List<vk::ExtensionProperties> availableExtensions(extensionCount);
		VkCall(vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data()));

		for (const auto& extension : availableExtensions)
		{
			extensions.push_back(extension.extensionName);
		}

		return extensions;
	}

	List<String> VulkanContext::QueryAvailableLayers()
	{
		List<String> layers;

		U32 layerCount = 0;
		VkCall(vk::enumerateInstanceLayerProperties(&layerCount, nullptr));

		List<vk::LayerProperties> availableLayers(layerCount);
		VkCall(vk::enumerateInstanceLayerProperties(&layerCount, availableLayers.data()));

		for (const auto& layer : availableLayers)
		{
			layers.push_back(layer.layerName);
		}

		return layers;
	}

	List<vk::PhysicalDevice> VulkanContext::QueryPhysicalDevices()
	{
		List<vk::PhysicalDevice> devices;

		U32 deviceCount = 0;
		VkCall(m_Instance.enumeratePhysicalDevices(&deviceCount, nullptr));

		if (deviceCount == 0)
		{
			log::Fatal("No Vulkan devices found");
		}

		devices.resize(deviceCount);
		VkCall(m_Instance.enumeratePhysicalDevices(&deviceCount, devices.data()));

		return devices;
	}

	vk::PhysicalDevice VulkanContext::PickPhysicalDevice()
	{
		const auto devices = QueryPhysicalDevices();

		for (const auto& device : devices)
		{
			if (device.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
			{
				return device;
			}
		}

		log::Fatal("No suitable Vulkan device found");
		return nullptr;
	}

	Bool VulkanContext::CreateInstance()
	{
		vk::ApplicationInfo appInfo = vk::ApplicationInfo()
			.setPApplicationName("TLC")
			.setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
			.setPEngineName("TLC")
			.setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
			.setApiVersion(VK_API_VERSION_1_2);

		vk::InstanceCreateInfo createInfo = vk::InstanceCreateInfo()
			.setPApplicationInfo(&appInfo)
			.setEnabledLayerCount(0);

		U32 glfwExtensionCount = 0;
		CString* glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		List<CString> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#if !defined(NDEBUG)
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		static const List<CString> validationLayersReq = {
			"VK_LAYER_KHRONOS_validation"
		};

		List<CString> validationLayers;

		const auto availableLayers = QueryAvailableLayers();

		for (const auto& layer : validationLayersReq)
		{
			if (std::find(availableLayers.begin(), availableLayers.end(), layer) == availableLayers.end())
			{
				log::Warn("Validation layer {} not available", layer);
			}
			else
			{
				validationLayers.push_back(layer);
			}
		}

		if (validationLayers.size() != validationLayersReq.size())
		{
			log::Warn("Not all validation layers available");
		}

		m_Layers = validationLayers;

		createInfo.setEnabledLayerCount(static_cast<U32>(validationLayers.size()))
			.setPpEnabledLayerNames(validationLayers.data());
#endif

		m_Extensions = extensions;

		createInfo.setEnabledExtensionCount(static_cast<U32>(extensions.size()))
			.setPpEnabledExtensionNames(extensions.data());

		VkCritCall(vk::createInstance(&createInfo, nullptr, &m_Instance));


#if !defined(NDEBUG)
		CreateDebugMessenger();
#endif

		return true;
	}

	void VulkanContext::CreateDebugMessenger()
	{
		auto createInfo = vk::DebugUtilsMessengerCreateInfoEXT()
			.setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
				| vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo
				| vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
				| vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
			.setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
				| vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
				| vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation)
			.setPfnUserCallback(VulkanDebugCallback)
			.setPUserData(this);

	
		// m_DebugMessenger = m_Instance.createDebugUtilsMessengerEXT(createInfo);
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)m_Instance.getProcAddr("vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(m_Instance, reinterpret_cast<const VkDebugUtilsMessengerCreateInfoEXT*>(&createInfo), nullptr, &m_DebugMessenger);
		}
		
		if (!m_DebugMessenger)
		{
			log::Warn("Failed to create Vulkan debug messenger");
		}
	}

	void VulkanContext::DestroyDebugMessenger()
	{
		// m_Instance.destroyDebugUtilsMessengerEXT(m_DebugMessenger);

		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)m_Instance.getProcAddr("vkDestroyDebugUtilsMessengerEXT");

		if (func != nullptr)
		{
			func(m_Instance, m_DebugMessenger, nullptr);
		}
		else
		{
			log::Warn("Failed to destroy Vulkan debug messenger");
		}

	}

	VulkanDevice* VulkanContext::CreateDevice(vk::PhysicalDevice physicalDevice, Bool requireGraphics, Bool requireCompute)
	{
		(void)physicalDevice; (void)requireGraphics; (void)requireCompute;
		//m_Devices.push_back(CreateScope<VulkanDevice>(physicalDevice, requireGraphics, requireCompute));
		//return m_Devices.back().get();
		return nullptr;
	}

}