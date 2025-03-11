#pragma once

#include "vulkanapi/VulkanBase.hpp"
#include "vulkanapi/VulkanDevice.hpp"
#include "vulkanapi/VulkanShader.hpp"
#include "vulkanapi/VulkanBuffer.hpp"
#include "vulkanapi/VulkanCommandBuffer.hpp"
#include "vulkanapi/VulkanSwapchain.hpp"
#include "vulkanapi/VulkanGraphicsPipeline.hpp"

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

		VulkanDevice* CreateDevice(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface = VK_NULL_HANDLE, const VulkanDeviceSettings& settings = VulkanDeviceSettings());
		vk::SurfaceKHR CreateSurface(Window* window);
		void DestroySurface(vk::SurfaceKHR surface);

		Raw<VulkanSwapchain> CreateSwapchain(Raw<Window> window, Raw<VulkanDevice> device, vk::SurfaceKHR surface);


		inline static VulkanContext* Get() { if (!s_Instance) s_Instance = CreateScope<VulkanContext>(); return s_Instance.get(); }
		inline static void Shutdown() { s_Instance.reset(); }

		inline const vk::Instance& GetInstance() const { return m_Instance; }

		inline const List<CString>& GetExtensions() const { return m_Extensions; }
		inline const List<CString>& GetLayers() const { return m_Layers; }

	private:
		Bool CreateInstance();

		void CreateDebugMessenger();
		void DestroyDebugMessenger();

	private:
		vk::Instance m_Instance = VK_NULL_HANDLE;

		Scope<VulkanSwapchain> m_Swapchain = nullptr;

		List<CString> m_Extensions;
		List<CString> m_Layers;
		List<Scope<VulkanDevice>> m_Devices;
		
		// vk::DebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;

		static Scope<VulkanContext> s_Instance;
	};
}