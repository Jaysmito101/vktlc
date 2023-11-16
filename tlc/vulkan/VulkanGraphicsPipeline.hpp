#pragma once

#include "vulkan/VulkanBase.hpp"

namespace tlc
{

	class VulkanContext;
	class VulkanDevice;
	class VulkanSwapchain;
	class VulkanShaderModule;

	struct VulkanGraphicsPipelineSettings
	{
		vk::Extent2D extent = vk::Extent2D(0, 0);
		Ref<VulkanShaderModule> vertexShaderModule;
		Ref<VulkanShaderModule> fragmentShaderModule;
		vk::RenderPass renderPass;

		VulkanGraphicsPipelineSettings() = default;
		VulkanGraphicsPipelineSettings(const VulkanGraphicsPipelineSettings&) = default;
		
		inline VulkanGraphicsPipelineSettings& SetExtent(const vk::Extent2D& ex) { this->extent = ex; return *this; }
		inline VulkanGraphicsPipelineSettings& SetVertexShaderModule(Ref<VulkanShaderModule> shModule) { this->vertexShaderModule = std::move(shModule); return *this; }
		inline VulkanGraphicsPipelineSettings& SetFragmentShaderModule(Ref<VulkanShaderModule> shModule) { this->fragmentShaderModule = std::move(shModule); return *this; }
		inline VulkanGraphicsPipelineSettings& SetRenderPass(vk::RenderPass r) { this->renderPass = r; return *this; }

	};

	struct VulkanGraphicsPipelineProperties
	{
		vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo;
		vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo;
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo;
		vk::Viewport viewport;
		vk::Rect2D scissor;
		vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo;
		vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo;
		vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo;
		vk::PipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo;
		vk::PipelineViewportStateCreateInfo viewportStateCreateInfo;
		vk::GraphicsPipelineCreateInfo pipelineCreateInfo;
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
	};

	class VulkanGraphicsPipeline
	{
	public:
		VulkanGraphicsPipeline(VulkanDevice* device, const VulkanGraphicsPipelineSettings& settings = VulkanGraphicsPipelineSettings());
		~VulkanGraphicsPipeline();

		Bool Recreate();

		inline const VulkanGraphicsPipelineSettings& GetSettings() const { return m_Settings; }
		inline VulkanGraphicsPipelineSettings& GetSettings() { return m_Settings; }

		inline Bool IsReady() { return m_IsReady; }

	private:

		vk::PipelineDynamicStateCreateInfo GetDynamicStateCreateInfo() const;
		vk::PipelineVertexInputStateCreateInfo GetVertexInputStateCreateInfo() const;
		vk::PipelineInputAssemblyStateCreateInfo GetInputAssemblyStateCreateInfo() const;
		Pair<vk::Viewport, vk::Rect2D> GetViewportAndScissor() const;
		vk::PipelineRasterizationStateCreateInfo GetRasterizationStateCreateInfo() const;
		vk::PipelineMultisampleStateCreateInfo GetMultisampleStateCreateInfo() const;
		vk::PipelineColorBlendAttachmentState GetColorBlendAttachmentState() const;
		vk::PipelineDepthStencilStateCreateInfo GetDepthStencilStateCreateInfo() const;
		vk::PipelineLayout CreatePipelineLayout() const;

		void Cleanup();

	private:
		VulkanContext* m_Context;
		VulkanDevice* m_Device;
		VulkanSwapchain* m_Swapchain;

		VulkanGraphicsPipelineSettings m_Settings;
		VulkanGraphicsPipelineProperties m_Properties;

		vk::Pipeline m_Pipeline = VK_NULL_HANDLE;
		vk::PipelineLayout m_PipelineLayout = VK_NULL_HANDLE;

		Bool m_IsReady = false;

	};

}