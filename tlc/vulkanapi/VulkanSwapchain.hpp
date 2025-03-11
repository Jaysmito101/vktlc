#pragma once

#include "vulkanapi/VulkanBase.hpp"

namespace tlc
{

	class VulkanDevice;
	class VulkanContext;
	class VulkanFramebuffer;
	class Window;

	class VulkanSwapchain
	{
	public:
		VulkanSwapchain(Raw<VulkanDevice> device, Raw<VulkanContext> context, Raw<Window> window, vk::SurfaceKHR surface);
		~VulkanSwapchain();
	
		U32 AcquireNextImage(vk::Semaphore semaphore = VK_NULL_HANDLE, vk::Fence fence = VK_NULL_HANDLE, U64 timeout = UINT64_MAX);
		void PresentImage(U32 index, vk::Semaphore waitSemaphore = VK_NULL_HANDLE);
	
		inline Bool IsReady() const { return m_IsReady; }
		inline const vk::SurfaceFormatKHR& GetSurfaceFormat() const { return m_SwapchainImageFormat; }
		inline const vk::Extent2D& GetExtent() const { return m_SwapchainExtent; }
		inline const List<vk::Image>& GetImages() const { return m_Images; }
		inline const List<vk::ImageView>& GetImageViews() const { return m_ImageViews; }
		
		Bool Recreate();
	
	private:
		Bool ChooseSufaceFormat();
		Bool ChoosePresentMode();
		Bool ChooseExtent();
		Bool QuerySwapchainImages();
		Bool CreateImageViews();

		void Cleanup(Bool forRecreate);

	private:
		Raw<VulkanContext> m_Context;
		Raw<VulkanDevice> m_Device;
		Raw<Window> m_Window;

		vk::SurfaceKHR m_Surface = VK_NULL_HANDLE;
		vk::SwapchainKHR m_Swapchain = VK_NULL_HANDLE;
		vk::SurfaceFormatKHR m_SwapchainImageFormat = vk::Format::eUndefined;
		vk::PresentModeKHR m_PresentMode = vk::PresentModeKHR::eFifo;
		vk::Extent2D m_SwapchainExtent = { 0, 0 };
		List<vk::Image> m_Images;
		List<vk::ImageView> m_ImageViews;

		Bool m_IsReady = false;
	};
}