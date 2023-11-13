#pragma once

#include "vulkan/VulkanBase.hpp"
#include "vulkan/VulkanDevice.hpp"

namespace tlc
{
	class VulkanContext
	{
	public:
		VulkanContext();
		virtual ~VulkanContext();

		List<String> QueryAvailableExtensions();
		List<String> QueryAvailableLayers();

		List<vk::PhysicalDevice> QueryPhysicalDevices();
		vk::PhysicalDevice PickPhysicalDevice();

		VulkanDevice* CreateDevice(vk::PhysicalDevice physicalDevice, Bool requireGraphics = true, Bool requireCompute = true);

		inline static VulkanContext* Get() { if (!s_Instance) s_Instance = CreateScope<VulkanContext>(); return s_Instance.get(); }
		inline static void Shutdown() { s_Instance.reset(); }

	private:
		Bool CreateInstance();

		void CreateDebugMessenger();
		void DestroyDebugMessenger();

	private:
		vk::Instance m_Instance = VK_NULL_HANDLE;
		vk::Device m_Device = VK_NULL_HANDLE;
		List<CString> m_Extensions;
		List<CString> m_Layers;
		List<Scope<VulkanDevice>> m_Devices;
		// vk::DebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;

		static Scope<VulkanContext> s_Instance;
	};
}