#include "vulkanapi/VulkanContext.hpp"
#include "vulkanapi/VulkanGraphicsPipeline.hpp"
#include "vulkanapi/VulkanShader.hpp"

namespace tlc
{

	VulkanGraphicsPipeline::VulkanGraphicsPipeline(VulkanDevice* device, const VulkanGraphicsPipelineSettings& settings)
	{
		m_Context = device->GetParentContext();
		m_Device = device;
		m_Settings = settings;

		Recreate();
	}

	VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
	{
		Cleanup();
	}

	Bool VulkanGraphicsPipeline::Recreate()
	{
		Cleanup();

		m_Properties.dynamicStateCreateInfo = GetDynamicStateCreateInfo();
		m_Properties.vertexInputStateCreateInfo = GetVertexInputStateCreateInfo();
		m_Properties.inputAssemblyStateCreateInfo = GetInputAssemblyStateCreateInfo();
		auto viewportAndScissor = GetViewportAndScissor();
		m_Properties.viewport = viewportAndScissor.first;
		m_Properties.scissor = viewportAndScissor.second;
		m_Properties.rasterizationStateCreateInfo = GetRasterizationStateCreateInfo();
		m_Properties.multisampleStateCreateInfo = GetMultisampleStateCreateInfo();
		m_Properties.depthStencilStateCreateInfo = GetDepthStencilStateCreateInfo();
		
		m_PipelineLayout = CreatePipelineLayout();

		m_Properties.viewportStateCreateInfo = vk::PipelineViewportStateCreateInfo()
			.setViewportCount(1)
			.setPViewports(&m_Properties.viewport)
			.setScissorCount(1)
			.setPScissors(&m_Properties.scissor);
		
		auto colorBlendAttachmentState = GetColorBlendAttachmentState();

		m_Properties.colorBlendStateCreateInfo = vk::PipelineColorBlendStateCreateInfo()
			.setLogicOpEnable(false)
			.setLogicOp(vk::LogicOp::eCopy)
			.setAttachmentCount(1)
			.setPAttachments(&colorBlendAttachmentState)
			.setBlendConstants({ 0.0f, 0.0f, 0.0f, 0.0f });

		m_Properties.shaderStages = {
			m_Settings.vertexShaderModule->GetShaderStageCreateInfo(vk::ShaderStageFlagBits::eVertex),
			m_Settings.fragmentShaderModule->GetShaderStageCreateInfo(vk::ShaderStageFlagBits::eFragment)
		};

		m_Properties.pipelineCreateInfo = vk::GraphicsPipelineCreateInfo()
			.setStageCount(static_cast<uint32_t>(m_Properties.shaderStages.size()))
			.setPStages(m_Properties.shaderStages.data())
			.setPVertexInputState(&m_Properties.vertexInputStateCreateInfo)
			.setPInputAssemblyState(&m_Properties.inputAssemblyStateCreateInfo)
			.setPViewportState(&m_Properties.viewportStateCreateInfo)
			.setPRasterizationState(&m_Properties.rasterizationStateCreateInfo)
			.setPMultisampleState(&m_Properties.multisampleStateCreateInfo)
			.setPDepthStencilState(&m_Properties.depthStencilStateCreateInfo)
			.setPColorBlendState(&m_Properties.colorBlendStateCreateInfo)
			.setPDynamicState(&m_Properties.dynamicStateCreateInfo)
			.setLayout(m_PipelineLayout)
			.setRenderPass(m_Settings.renderPass)
			.setSubpass(0)
			.setBasePipelineHandle(VK_NULL_HANDLE)
			.setBasePipelineIndex(-1);
		
 		auto res = m_Device->GetDevice().createGraphicsPipeline(nullptr, m_Properties.pipelineCreateInfo);

		if (res.result != vk::Result::eSuccess)
		{
			log::Error("Failed to create graphics pipeline");
			return false;
		}

		m_Pipeline = res.value;

		m_IsReady = true;

		return true;
	}

	vk::PipelineDynamicStateCreateInfo VulkanGraphicsPipeline::GetDynamicStateCreateInfo() const
	{
		static List<vk::DynamicState> dynamicStates = {
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor
		};

		auto dynamicStateCreateInfo = vk::PipelineDynamicStateCreateInfo()
			.setDynamicStateCount(static_cast<uint32_t>(dynamicStates.size()))
			.setPDynamicStates(dynamicStates.data());

		return dynamicStateCreateInfo;
	}

	vk::PipelineVertexInputStateCreateInfo VulkanGraphicsPipeline::GetVertexInputStateCreateInfo()
	{
		vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
		if (m_Settings.vertexInputAttributeDescriptions)
		{
			m_Properties.vertexInputBindingDescription = vk::VertexInputBindingDescription()
				.setBinding(0)
				.setStride(m_Settings.vertexInputBindingDescriptionStride)
				.setInputRate(vk::VertexInputRate::eVertex);
			m_Properties.vertexInputAttributeDescriptions = *m_Settings.vertexInputAttributeDescriptions;
			vertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo()
				.setVertexBindingDescriptionCount(1)
				.setPVertexBindingDescriptions(&m_Properties.vertexInputBindingDescription)
				.setVertexAttributeDescriptionCount(static_cast<uint32_t>(m_Properties.vertexInputAttributeDescriptions.size()))
				.setPVertexAttributeDescriptions(m_Properties.vertexInputAttributeDescriptions.data());
		} else {
			vertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo()
				.setVertexBindingDescriptionCount(0)
				.setPVertexBindingDescriptions(nullptr)
				.setVertexAttributeDescriptionCount(0)
				.setPVertexAttributeDescriptions(nullptr);
		}
		return vertexInputStateCreateInfo;
	}

	vk::PipelineInputAssemblyStateCreateInfo VulkanGraphicsPipeline::GetInputAssemblyStateCreateInfo() const
	{
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = vk::PipelineInputAssemblyStateCreateInfo()
			.setTopology(vk::PrimitiveTopology::eTriangleList)
			.setPrimitiveRestartEnable(false);

		return inputAssemblyStateCreateInfo;
	}

	Pair<vk::Viewport, vk::Rect2D> VulkanGraphicsPipeline::GetViewportAndScissor() const
	{
		auto viewport = vk::Viewport()
			.setX(0.0f)
			.setY(0.0f)
			.setWidth(static_cast<float>(m_Settings.extent.width))
			.setHeight(static_cast<float>(m_Settings.extent.height))
			.setMinDepth(0.0f)
			.setMaxDepth(1.0f);

		vk::Rect2D scissor = vk::Rect2D()
			.setOffset({ 0, 0 })
			.setExtent(m_Settings.extent);

		return { viewport, scissor };
	}

	vk::PipelineRasterizationStateCreateInfo VulkanGraphicsPipeline::GetRasterizationStateCreateInfo() const
	{
		auto rasterizationStateCreateInfo = vk::PipelineRasterizationStateCreateInfo()
			.setDepthClampEnable(false)
			.setRasterizerDiscardEnable(false)
			.setPolygonMode(vk::PolygonMode::eFill)
			.setLineWidth(1.0f)
			.setCullMode(vk::CullModeFlagBits::eNone)
			.setFrontFace(vk::FrontFace::eClockwise)
			.setDepthBiasEnable(false)
			.setDepthBiasConstantFactor(0.0f)
			.setDepthBiasClamp(0.0f)
			.setDepthBiasSlopeFactor(0.0f);

		return rasterizationStateCreateInfo;
	}

	vk::PipelineMultisampleStateCreateInfo VulkanGraphicsPipeline::GetMultisampleStateCreateInfo() const
	{
		auto multisampleStateCreateInfo = vk::PipelineMultisampleStateCreateInfo()
			.setSampleShadingEnable(false)
			.setRasterizationSamples(vk::SampleCountFlagBits::e1)
			.setMinSampleShading(1.0f)
			.setPSampleMask(nullptr)
			.setAlphaToCoverageEnable(false)
			.setAlphaToOneEnable(false);

		return multisampleStateCreateInfo;
	}

	vk::PipelineColorBlendAttachmentState VulkanGraphicsPipeline::GetColorBlendAttachmentState() const
	{
		if (m_Settings.blendAttachmentState.has_value()) {
			return m_Settings.blendAttachmentState.value();
		}

		auto colorBlendAttachmentState = vk::PipelineColorBlendAttachmentState()
			.setColorWriteMask(
				vk::ColorComponentFlagBits::eR |
				vk::ColorComponentFlagBits::eG |
				vk::ColorComponentFlagBits::eB |
				vk::ColorComponentFlagBits::eA)
			.setBlendEnable(true)
			.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
			.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
			.setColorBlendOp(vk::BlendOp::eAdd)
			.setSrcAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcColor)
			.setDstAlphaBlendFactor(vk::BlendFactor::eZero)
			.setAlphaBlendOp(vk::BlendOp::eAdd);
		return colorBlendAttachmentState;
	}

	vk::PipelineDepthStencilStateCreateInfo VulkanGraphicsPipeline::GetDepthStencilStateCreateInfo() const
	{
		auto depthStencilStateCreateInfo = vk::PipelineDepthStencilStateCreateInfo()
			.setDepthTestEnable(false)
			.setDepthWriteEnable(false)
			.setDepthCompareOp(vk::CompareOp::eLess)
			.setDepthBoundsTestEnable(false)
			.setMinDepthBounds(0.0f)
			.setMaxDepthBounds(1.0f)
			.setStencilTestEnable(false)
			.setFront({})
			.setBack({});

		return depthStencilStateCreateInfo;
	}

	vk::PipelineLayout VulkanGraphicsPipeline::CreatePipelineLayout() const
	{
		auto pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
			.setSetLayoutCount(static_cast<U32>(m_Settings.descriptorSetLayouts.size()))
			.setPSetLayouts(m_Settings.descriptorSetLayouts.data())
			.setPushConstantRangeCount(static_cast<U32>(m_Settings.pushConstantRanges.size()))
			.setPPushConstantRanges(m_Settings.pushConstantRanges.data());

		auto [result, pipelineLayout] = m_Device->GetDevice().createPipelineLayout(pipelineLayoutCreateInfo);
		VkCall(result);
		return pipelineLayout;
	}

	void VulkanGraphicsPipeline::Cleanup()
	{
		m_Device->WaitIdle();

		if (m_IsReady)
		{
			m_Device->GetDevice().destroyPipeline(m_Pipeline);
			m_Device->GetDevice().destroyPipelineLayout(m_PipelineLayout);
		}

		m_IsReady = false;
	}


}