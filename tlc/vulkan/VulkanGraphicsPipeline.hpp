#pragma once

#include "vulkan/VulkanBase.hpp"

namespace tlc
{

	class VulkanContext;
	class VulkanDevice;
	class VulkanSwapchain;

	class VulkanGraphicsPipeline
	{
	public:
		VulkanGraphicsPipeline(VulkanDevice* device, VulkanSwapchain* swapchain = nullptr);
		~VulkanGraphicsPipeline();

		void Recreate(vk::Extent2D swapchainExtent);

	private:

		vk::PipelineDynamicStateCreateInfo GetDynamicStateCreateInfo() const;
		vk::PipelineVertexInputStateCreateInfo GetVertexInputStateCreateInfo() const;
		vk::PipelineInputAssemblyStateCreateInfo GetInputAssemblyStateCreateInfo() const;
		Pair<vk::Viewport, vk::Rect2D> GetViewportAndScissor(vk::Extent2D extent) const;
		vk::PipelineRasterizationStateCreateInfo GetRasterizationStateCreateInfo() const;
		vk::PipelineMultisampleStateCreateInfo GetMultisampleStateCreateInfo() const;
		vk::PipelineColorBlendAttachmentState GetColorBlendAttachmentState() const;
		vk::PipelineColorBlendStateCreateInfo GetColorBlendStateCreateInfo() const;
		vk::PipelineDepthStencilStateCreateInfo GetDepthStencilStateCreateInfo() const;
		vk::PipelineLayout CreatePipelineLayout() const;
		vk::RenderPass CreateRenderPass() const;

		void Cleanup();

	private:
		VulkanContext* m_Context;
		VulkanDevice* m_Device;
		VulkanSwapchain* m_Swapchain;

		vk::RenderPass m_RenderPass = VK_NULL_HANDLE;
		vk::Pipeline m_GraphicsPipeline = VK_NULL_HANDLE;
		vk::PipelineLayout m_PipelineLayout = VK_NULL_HANDLE;

		Bool m_IsReady = false;

	};

}