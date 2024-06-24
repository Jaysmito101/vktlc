#pragma once

#include "vulkan/VulkanBase.hpp"

namespace tlc
{

	class VulkanContext;
	class VulkanDevice;
	class VulkanSwapchain;
	class VulkanFramebuffer;
	class VulkanShaderModule;

	struct VulkanVertex
	{
		glm::vec4 position = glm::vec4(0.0f);
		glm::vec4 normal = glm::vec4(0.0f);
		glm::vec4 texCoord = glm::vec4(0.0f);
		glm::vec4 color = glm::vec4(0.0f);
		glm::vec4 boneWeights = glm::vec4(0.0f);
		glm::ivec4 boneIDs = glm::ivec4(-1);

		inline static List<vk::VertexInputAttributeDescription> GetAttributeDescriptions()
		{
			List<vk::VertexInputAttributeDescription> attributeDescriptions;
			attributeDescriptions.resize(6);

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = vk::Format::eR32G32B32A32Sfloat;
			attributeDescriptions[0].offset = offsetof(VulkanVertex, position);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = vk::Format::eR32G32B32A32Sfloat;
			attributeDescriptions[1].offset = offsetof(VulkanVertex, normal);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = vk::Format::eR32G32B32A32Sfloat;
			attributeDescriptions[2].offset = offsetof(VulkanVertex, texCoord);

			attributeDescriptions[3].binding = 0;
			attributeDescriptions[3].location = 3;
			attributeDescriptions[3].format = vk::Format::eR32G32B32A32Sfloat;
			attributeDescriptions[3].offset = offsetof(VulkanVertex, color);

			attributeDescriptions[4].binding = 0;
			attributeDescriptions[4].location = 4;
			attributeDescriptions[4].format = vk::Format::eR32G32B32A32Sfloat;
			attributeDescriptions[4].offset = offsetof(VulkanVertex, boneWeights);

			attributeDescriptions[5].binding = 0;
			attributeDescriptions[5].location = 5;
			attributeDescriptions[5].format = vk::Format::eR32G32B32A32Sint;
			attributeDescriptions[5].offset = offsetof(VulkanVertex, boneIDs);

			return attributeDescriptions;
		}
	};

	struct VulkanGraphicsPipelineSettings
	{
		vk::Extent2D extent = vk::Extent2D(0, 0);
		Ref<VulkanShaderModule> vertexShaderModule;
		Ref<VulkanShaderModule> fragmentShaderModule;
		vk::RenderPass renderPass;

		VulkanFramebuffer* framebuffer = nullptr;

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
		List<vk::PipelineShaderStageCreateInfo> shaderStages;
		vk::VertexInputBindingDescription vertexInputBindingDescription;
		List<vk::VertexInputAttributeDescription> vertexInputAttributeDescriptions;
	};

	class VulkanGraphicsPipeline
	{
	public:
		VulkanGraphicsPipeline(VulkanDevice* device, const VulkanGraphicsPipelineSettings& settings = VulkanGraphicsPipelineSettings());
		~VulkanGraphicsPipeline();

		Bool Recreate();

		inline const VulkanGraphicsPipelineSettings& GetSettings() const { return m_Settings; }
		inline VulkanGraphicsPipelineSettings& GetSettings() { return m_Settings; }

		inline Bool IsReady() const { return m_IsReady; }
		inline vk::Pipeline GetPipeline() const { return m_Pipeline; }
		inline vk::PipelineLayout GetPipelineLayout() const { return m_PipelineLayout; }

		friend class VulkanFramebuffer;

	private:

		vk::PipelineVertexInputStateCreateInfo GetVertexInputStateCreateInfo();
		vk::PipelineDynamicStateCreateInfo GetDynamicStateCreateInfo() const;
		vk::PipelineInputAssemblyStateCreateInfo GetInputAssemblyStateCreateInfo() const;
		Pair<vk::Viewport, vk::Rect2D> GetViewportAndScissor() const;
		vk::PipelineRasterizationStateCreateInfo GetRasterizationStateCreateInfo() const;
		vk::PipelineMultisampleStateCreateInfo GetMultisampleStateCreateInfo() const;
		vk::PipelineColorBlendAttachmentState GetColorBlendAttachmentState() const;
		vk::PipelineDepthStencilStateCreateInfo GetDepthStencilStateCreateInfo() const;
		vk::PipelineLayout CreatePipelineLayout() const;

		void Cleanup();

	private:
		Raw<VulkanContext> m_Context;
		Raw<VulkanDevice> m_Device;
		Raw<VulkanSwapchain> m_Swapchain;

		VulkanGraphicsPipelineSettings m_Settings;
		VulkanGraphicsPipelineProperties m_Properties;

		vk::Pipeline m_Pipeline = VK_NULL_HANDLE;
		vk::PipelineLayout m_PipelineLayout = VK_NULL_HANDLE;

		Bool m_IsReady = false;

	};

}