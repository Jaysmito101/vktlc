#pragma once

#include "vulkan/VulkanBase.hpp"

namespace tlc
{

	class VulkanDevice;
	class VulkanContext;
	class Window;

	class VulkanSwapchain
	{
	public:
		VulkanSwapchain(VulkanDevice* device, Window* window);
		~VulkanSwapchain();

		inline Bool GetIsReady() const { return m_IsReady; }
		inline const vk::SurfaceFormatKHR& GetSurfaceFormat() const { return m_SwapchainImageFormat; }
		inline const vk::Extent2D& GetExtent() const { return m_SwapchainExtent; }

	private:
		Bool ChooseSufaceFormat();
		Bool ChoosePresentMode();
		Bool ChooseExtent();
		Bool RecreateSwapchain();
		Bool QuerySwapchainImages();
		Bool CreateImageViews();

		void Cleanup();

	private:
		VulkanContext* m_Context;
		VulkanDevice* m_Device;
		Window* m_Window;

		vk::SwapchainKHR m_Swapchain = VK_NULL_HANDLE;
		vk::SurfaceFormatKHR m_SwapchainImageFormat = vk::Format::eUndefined;
		vk::PresentModeKHR m_PresentMode = vk::PresentModeKHR::eFifo;
		vk::Extent2D m_SwapchainExtent = { 0, 0 };
		List<vk::Image> m_Images;
		List<vk::ImageView> m_ImageViews;

		Bool m_IsReady = false;

		static Scope<VulkanSwapchain> s_Instance;
	};
}