#include "vulkan/VulkanContext.hpp"
#include "vulkan/VulkanGraphicsPipeline.hpp"
#include "vulkan/VulkanSwapchain.hpp"
#include "vulkan/VulkanShader.hpp"

namespace tlc
{



	VulkanGraphicsPipeline::VulkanGraphicsPipeline(VulkanDevice* device, VulkanSwapchain* swapchain = nullptr)
	{
		m_Context = device->GetParentContext();
		m_Device = device;
		m_Swapchain = swapchain;
	}

	VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
	{
		Cleanup();
	}

	void VulkanGraphicsPipeline::Recreate(vk::Extent2D swapchainExtent)
	{

		auto dynamicStateCreateInfo = GetDynamicStateCreateInfo();
		auto vertexInputStateCreateInfo = GetVertexInputStateCreateInfo();
		auto inputAssemblyStateCreateInfo = GetInputAssemblyStateCreateInfo();
		auto [viewport, scissor] = GetViewportAndScissor(swapchainExtent);
		auto rasterizationStateCreateInfo = GetRasterizationStateCreateInfo();	
		auto multisampleStateCreateInfo = GetMultisampleStateCreateInfo();
		auto colorBlendStateCreateInfo = GetColorBlendStateCreateInfo();
		auto depthStencilStateCreateInfo = GetDepthStencilStateCreateInfo();

		m_PipelineLayout = CreatePipelineLayout();
		m_RenderPass = CreateRenderPass();

		auto viewportStateCreateInfo = vk::PipelineViewportStateCreateInfo()
			.setViewportCount(1)
			.setPViewports(&viewport)
			.setScissorCount(1)
			.setPScissors(&scissor);


		auto pipelineCreateInfo = vk::GraphicsPipelineCreateInfo()
			// .setStageCount(static_cast<uint32_t>(m_ShaderStages.size()))
			// .setPStages(m_ShaderStages.data())

			.setPVertexInputState(&vertexInputStateCreateInfo)
			.setPInputAssemblyState(&inputAssemblyStateCreateInfo)
			.setPViewportState(&viewportStateCreateInfo)
			.setPRasterizationState(&rasterizationStateCreateInfo)
			.setPMultisampleState(&multisampleStateCreateInfo)
			.setPDepthStencilState(&depthStencilStateCreateInfo)
			.setPColorBlendState(&colorBlendStateCreateInfo)
			.setPDynamicState(&dynamicStateCreateInfo)
			.setLayout(m_PipelineLayout)
			.setRenderPass(m_RenderPass)
			.setSubpass(0)
			.setBasePipelineHandle(nullptr)
			.setBasePipelineIndex(-1);
		
		auto [result, pipeline] = m_Device->GetDevice().createGraphicsPipeline(nullptr, pipelineCreateInfo);

		if (result != vk::Result::eSuccess)
		{
			log::Error("Failed to create graphics pipeline");
			return;
		}

		m_Pipeline = pipeline;
	}

	vk::PipelineDynamicStateCreateInfo VulkanGraphicsPipeline::GetDynamicStateCreateInfo() const
	{
		List<vk::DynamicState> dynamicStates = {
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor
		};

		auto dynamicStateCreateInfo = vk::PipelineDynamicStateCreateInfo()
			.setDynamicStateCount(static_cast<uint32_t>(dynamicStates.size()))
			.setPDynamicStates(dynamicStates.data());

		return dynamicStateCreateInfo;
	}

	vk::PipelineVertexInputStateCreateInfo VulkanGraphicsPipeline::GetVertexInputStateCreateInfo() const
	{
		auto vertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo()
			.setVertexBindingDescriptionCount(0)
			.setPVertexBindingDescriptions(nullptr)
			.setVertexAttributeDescriptionCount(0)
			.setPVertexAttributeDescriptions(nullptr);

		return vertexInputStateCreateInfo;
	}

	vk::PipelineInputAssemblyStateCreateInfo VulkanGraphicsPipeline::GetInputAssemblyStateCreateInfo() const
	{
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = vk::PipelineInputAssemblyStateCreateInfo()
			.setTopology(vk::PrimitiveTopology::eTriangleList)
			.setPrimitiveRestartEnable(false);

		return inputAssemblyStateCreateInfo;
	}

	Pair<vk::Viewport, vk::Rect2D> VulkanGraphicsPipeline::GetViewportAndScissor(vk::Extent2D swapchainExtent) const
	{
		auto viewport = vk::Viewport()
			.setX(0.0f)
			.setY(0.0f)
			.setWidth(static_cast<float>(swapchainExtent.width))
			.setHeight(static_cast<float>(swapchainExtent.height))
			.setMinDepth(0.0f)
			.setMaxDepth(1.0f);

		vk::Rect2D scissor = vk::Rect2D()
			.setOffset({ 0, 0 })
			.setExtent(swapchainExtent);

		return { viewport, scissor };
	}

	vk::PipelineRasterizationStateCreateInfo VulkanGraphicsPipeline::GetRasterizationStateCreateInfo() const
	{
		auto rasterizationStateCreateInfo = vk::PipelineRasterizationStateCreateInfo()
			.setDepthClampEnable(false)
			.setRasterizerDiscardEnable(false)
			.setPolygonMode(vk::PolygonMode::eFill)
			.setLineWidth(1.0f)
			.setCullMode(vk::CullModeFlagBits::eBack)
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
			.setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
			.setDstAlphaBlendFactor(vk::BlendFactor::eZero)
			.setAlphaBlendOp(vk::BlendOp::eAdd);


		return colorBlendAttachmentState;
	}

	vk::PipelineColorBlendStateCreateInfo VulkanGraphicsPipeline::GetColorBlendStateCreateInfo() const
	{
		auto colorBlendAttachmentState = GetColorBlendAttachmentState();

		auto colorBlendStateCreateInfo = vk::PipelineColorBlendStateCreateInfo()
			.setLogicOpEnable(false)
			.setLogicOp(vk::LogicOp::eCopy)
			.setAttachmentCount(1)
			.setPAttachments(&colorBlendAttachmentState)
			.setBlendConstants({ 0.0f, 0.0f, 0.0f, 0.0f });

		return colorBlendStateCreateInfo;
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
			.setSetLayoutCount(0)
			.setPSetLayouts(nullptr)
			.setPushConstantRangeCount(0)
			.setPPushConstantRanges(nullptr);

		return m_Device->GetDevice().createPipelineLayout(pipelineLayoutCreateInfo);
	}

	vk::RenderPass VulkanGraphicsPipeline::CreateRenderPass() const
	{
		auto attachment = vk::AttachmentDescription()
			.setFormat(m_Swapchain->GetSurfaceFormat().format)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

		auto subpassAttachment = vk::AttachmentReference()
			.setAttachment(0)
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

		auto subpass = vk::SubpassDescription()
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachmentCount(1)
			.setPColorAttachments(&subpassAttachment)
			.setPDepthStencilAttachment(nullptr)
			.setPInputAttachments(nullptr)
			.setPreserveAttachmentCount(0)
			.setPResolveAttachments(nullptr);

		auto renderPassCreateInfo = vk::RenderPassCreateInfo()
			.setAttachmentCount(1)
			.setPAttachments(&attachment)
			.setSubpassCount(1)
			.setPSubpasses(&subpass)
			.setDependencyCount(0)
			.setPDependencies(nullptr);

		return m_Device->GetDevice().createRenderPass(renderPassCreateInfo);
	}

	void VulkanGraphicsPipeline::Cleanup()
	{
		if (m_IsReady)
		{
			m_Device->GetDevice().destroyPipelineLayout(m_PipelineLayout);
			m_Device->GetDevice().destroyRenderPass(m_RenderPass);
		}

		m_IsReady = false;
	}


}