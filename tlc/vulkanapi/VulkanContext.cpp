#include "vulkanapi/VulkanContext.hpp"
#include "core/Window.hpp"

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


#if !defined(NDEBUG)
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
#endif

		if (!CreateInstance())
		{
			log::Fatal("Failed to create Vulkan instance");
		}

#if !defined(NDEBUG)
		const auto physicalDevice = QueryPhysicalDevices();
		log::Trace("Available Vulkan devices:");
		for (const auto& device : physicalDevice)
		{
			log::Trace("\tDevice: {}", device.getProperties().deviceName.data());
		}
#endif
		
		log::Info("VulkanContext created");
	}

	VulkanContext::~VulkanContext()
	{
		log::Debug("Shutting down VulkanContext");

		m_Swapchain.reset();

		for (I32 i = 0 ; i < m_Devices.size() ; i++)
		{
			m_Devices[i].reset();
		}
		m_Devices.clear();

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

	// TODO: Make this more robust
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

	VulkanDevice* VulkanContext::CreateDevice(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, const VulkanDeviceSettings& settings)
	{
		m_Devices.push_back(CreateScope<VulkanDevice>(this, physicalDevice, settings, surface));
		return m_Devices.back().get();
	}

	vk::SurfaceKHR VulkanContext::CreateSurface(Window* window)
	{
		TLC_ASSERT(window, "Window is null");
		TLC_ASSERT(m_Instance, "Vulkan instance is null");	

		log::Debug("Creating Window Surface");
		VkSurfaceKHR surface = VK_NULL_HANDLE;
		if (glfwCreateWindowSurface(m_Instance, window->GetHandle(), nullptr, &surface) != VK_SUCCESS)
		{
			log::Fatal("Failed to create window surface");
		}
		log::Info("Window Surface created");
		return surface;
	}

	void VulkanContext::DestroySurface(vk::SurfaceKHR surface)
	{
		TLC_ASSERT(m_Instance, "Vulkan instance is null");
		vk::SurfaceKHR surfaceKHR = vk::SurfaceKHR(surface);
		m_Instance.destroySurfaceKHR(surfaceKHR);
	}

	Raw<VulkanSwapchain> VulkanContext::CreateSwapchain(Raw<Window> window, Raw<VulkanDevice> device, vk::SurfaceKHR surface)
	{
		if (!m_Swapchain)
		{
			m_Swapchain = CreateScope<VulkanSwapchain>(device, this, window, surface);
		}
		return m_Swapchain.get();
	}
}