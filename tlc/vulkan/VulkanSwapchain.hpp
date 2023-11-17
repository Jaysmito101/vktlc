#pragma once

#include "vulkan/VulkanBase.hpp"

namespace tlc
{

	class VulkanDevice;
	class VulkanContext;
	class VulkanFramebuffer;
	class Window;

	class VulkanSwapchain
	{
	public:
		VulkanSwapchain(VulkanDevice* device, Window* window);
		~VulkanSwapchain();

		U32 AcquireNextImage(vk::Semaphore semaphore = VK_NULL_HANDLE, vk::Fence fence = VK_NULL_HANDLE, U64 timeout = UINT64_MAX);
		void PresentImage(U32 index, vk::Semaphore waitSemaphore = VK_NULL_HANDLE);

		inline Bool IsReady() const { return m_IsReady; }
		inline const vk::SurfaceFormatKHR& GetSurfaceFormat() const { return m_SwapchainImageFormat; }
		inline const vk::Extent2D& GetExtent() const { return m_SwapchainExtent; }
		inline const vk::RenderPass GetRenderPass() const { return m_RenderPass; }
		inline const List<vk::Image>& GetImages() const { return m_Images; }
		inline const List<vk::ImageView>& GetImageViews() const { return m_ImageViews; }
		inline const List<Ref<VulkanFramebuffer>>& GetFramebuffers() const { return m_Framebuffers; }

	private:
		Bool ChooseSufaceFormat();
		Bool ChoosePresentMode();
		Bool ChooseExtent();
		Bool RecreateSwapchain();
		Bool QuerySwapchainImages();
		Bool CreateImageViews();
		Bool CreateRenderPass();
		Bool CreateFramebuffers();

		void Cleanup();

	private:
		VulkanContext* m_Context;
		VulkanDevice* m_Device;
		Window* m_Window;

		vk::RenderPass m_RenderPass = VK_NULL_HANDLE;
		vk::SwapchainKHR m_Swapchain = VK_NULL_HANDLE;
		vk::SurfaceFormatKHR m_SwapchainImageFormat = vk::Format::eUndefined;
		vk::PresentModeKHR m_PresentMode = vk::PresentModeKHR::eFifo;
		vk::Extent2D m_SwapchainExtent = { 0, 0 };
		List<vk::Image> m_Images;
		List<vk::ImageView> m_ImageViews;
		List<Ref<VulkanFramebuffer>> m_Framebuffers;

		Bool m_IsReady = false;

		static Scope<VulkanSwapchain> s_Instance;
	};
}