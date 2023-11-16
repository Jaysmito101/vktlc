#include "vulkan/VulkanFramebuffer.hpp"
#include "vulkan/VulkanDevice.hpp"
#include "vulkan/VulkanSwapchain.hpp"

namespace tlc
{




	VulkanFramebuffer::VulkanFramebuffer(VulkanDevice* device, const VulkanFramebufferSettings& settings)
	{
		m_Device = device;
		m_Settings = settings;

		m_IsRenderPassOwned = settings.renderPass == static_cast<vk::RenderPass>(VK_NULL_HANDLE);
		m_RenderPass = settings.renderPass;

		Recreate();
	}

	VulkanFramebuffer::~VulkanFramebuffer()
	{
		Cleanup();
	}

	Bool VulkanFramebuffer::Recreate()
	{
		Cleanup();

		if (m_IsRenderPassOwned)
		{
			if (!RecreateRenderPass())
			{
				log::Error("VulkanFramebuffer::Recreate: failed to recreate render pass");
				return false;
			}
		}

		vk::ImageView attachments[] = {
			m_Settings.swapchain->GetImageViews()[m_Settings.swapchainImageIndex]
		};

		auto framebufferCreateInfo = vk::FramebufferCreateInfo()
			.setRenderPass(m_RenderPass)
			.setAttachmentCount(1)
			.setPAttachments(&m_Settings.swapchain->GetImageViews()[m_Settings.swapchainImageIndex])
			.setWidth(m_Settings.swapchain->GetExtent().width)
			.setHeight(m_Settings.swapchain->GetExtent().height)
			.setLayers(1);

		auto result = m_Device->GetDevice().createFramebuffer(framebufferCreateInfo);

		if (result == static_cast<vk::Framebuffer>(VK_NULL_HANDLE))
		{
			log::Error("VulkanFramebuffer::Recreate: failed to create framebuffer");
			return false;
		}

		m_Framebuffer = result;

		m_IsReady = true;

		return true;
	}

	Ref<VulkanGraphicsPipeline> VulkanFramebuffer::CreateGraphicsPipeline(VulkanGraphicsPipelineSettings settings)
	{
		if (!m_IsReady)
		{
			log::Error("VulkanFramebuffer::CreateGraphicsPipeline: framebuffer is not ready");
			return nullptr;
		}

		settings.renderPass = m_RenderPass;
		settings.extent = m_Settings.swapchain->GetExtent();

		return CreateRef<VulkanGraphicsPipeline>(m_Device, settings);
	}

	void VulkanFramebuffer::Cleanup()
	{
		if (!m_IsReady)
		{
			return;
		}

		if (m_IsRenderPassOwned)
		{
			m_Device->GetDevice().destroyRenderPass(m_RenderPass);
		}

		m_Device->GetDevice().destroyFramebuffer(m_Framebuffer);

		m_IsReady = false;
	}

	Bool VulkanFramebuffer::RecreateRenderPass()
	{

		auto attachment = vk::AttachmentDescription()
			.setFormat(m_Settings.swapchain->GetSurfaceFormat().format)
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
		
		m_RenderPass = m_Device->GetDevice().createRenderPass(renderPassCreateInfo);

		if (m_RenderPass == static_cast<vk::RenderPass>(VK_NULL_HANDLE))
		{
			log::Error("VulkanFramebuffer::RecreateRenderPass: failed to create render pass");
			return false;
		}

		return true;
	}

}