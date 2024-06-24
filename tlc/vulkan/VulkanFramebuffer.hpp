#pragma once

#include "vulkan/VulkanBase.hpp"
#include "vulkan/VulkanGraphicsPipeline.hpp"


namespace tlc
{


	class VulkanDevice;
	class VulkanSwapchain;

	struct VulkanFramebufferSettings
	{
		VulkanSwapchain* swapchain = nullptr;
		I32 swapchainImageIndex = -1;
		vk::RenderPass renderPass = VK_NULL_HANDLE;


		inline VulkanFramebufferSettings& SetSwapchain(VulkanSwapchain* s) { this->swapchain = s; return *this; }
		inline VulkanFramebufferSettings& SetSwapchainImageIndex(I32 i) { this->swapchainImageIndex = i; return *this; }
		inline VulkanFramebufferSettings& SetRenderPass(vk::RenderPass r) { this->renderPass = r; return *this; }
	};

	class VulkanFramebuffer
	{
	public:
		VulkanFramebuffer(VulkanDevice* device, const VulkanFramebufferSettings& settings);
		~VulkanFramebuffer();

		Bool Recreate();
		
		Ref<VulkanGraphicsPipeline> CreateGraphicsPipeline(VulkanGraphicsPipelineSettings settings = VulkanGraphicsPipelineSettings());

		inline VulkanFramebufferSettings& GetSettings() { return m_Settings; }
		inline const vk::Framebuffer GetFramebuffer() const { return m_Framebuffer; }
		inline Bool IsReady() const { return m_IsReady; }

		friend class VulkanGraphicsPipeline;
	private:
		void Cleanup();
		Bool RecreateRenderPass();

	private:
		Raw<VulkanDevice> m_Device;

		VulkanFramebufferSettings m_Settings;

		Bool m_IsRenderPassOwned = false;

		vk::RenderPass m_RenderPass = VK_NULL_HANDLE;
		vk::Framebuffer m_Framebuffer = VK_NULL_HANDLE;
		Bool m_IsReady = false;
	};

}