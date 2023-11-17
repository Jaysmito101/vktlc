#pragma once

#include "vulkan/VulkanBase.hpp"
#include "vulkan/VulkanDevice.hpp"
#include "vulkan/VulkanShader.hpp"
#include "vulkan/VulkanCommandBuffer.hpp"
#include "vulkan/VulkanGraphicsPipeline.hpp"

namespace tlc
{
	class Window;

	class VulkanContext
	{
	public:
		VulkanContext();
		virtual ~VulkanContext();

		List<String> QueryAvailableExtensions();
		List<String> QueryAvailableLayers();

		List<vk::PhysicalDevice> QueryPhysicalDevices();
		vk::PhysicalDevice PickPhysicalDevice();

		VulkanDevice* CreateDevice(vk::PhysicalDevice physicalDevice, const VulkanDeviceSettings& settings = VulkanDeviceSettings());
		const vk::SurfaceKHR& CreateSurface(Window* window);

		inline static VulkanContext* Get() { if (!s_Instance) s_Instance = CreateScope<VulkanContext>(); return s_Instance.get(); }
		inline static void Shutdown() { s_Instance.reset(); }

		inline const vk::Instance& GetInstance() const { return m_Instance; }

		inline const List<CString>& GetExtensions() const { return m_Extensions; }
		inline const List<CString>& GetLayers() const { return m_Layers; }

		inline const vk::SurfaceKHR& GetSurface() const { return m_Surface; }

	private:
		Bool CreateInstance();

		void CreateDebugMessenger();
		void DestroyDebugMessenger();

	private:
		vk::Instance m_Instance = VK_NULL_HANDLE;
		vk::Device m_Device = VK_NULL_HANDLE;
		vk::SurfaceKHR m_Surface = VK_NULL_HANDLE;

		List<CString> m_Extensions;
		List<CString> m_Layers;
		List<Scope<VulkanDevice>> m_Devices;
		// vk::DebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;

		static Scope<VulkanContext> s_Instance;
	};
}