#include "vulkanapi/VulkanSwapchain.hpp"
#include "vulkanapi/vulkanContext.hpp"
#include "core/Window.hpp"

namespace tlc
{
	VulkanSwapchain::VulkanSwapchain(Raw<VulkanDevice> device, Raw<VulkanContext> context, Raw<Window> window, vk::SurfaceKHR surface)
		: m_Context(context), m_Device(device), m_Window(window), m_Surface(surface)
	{
		Recreate();
	}

	VulkanSwapchain::~VulkanSwapchain()
	{
		log::Debug("Destroying swapchain");
		if (m_IsReady)
		{
			Cleanup(false);
		}

		if (m_Surface != static_cast<vk::SurfaceKHR>(VK_NULL_HANDLE))
		{
			log::Debug("Destroying Window Surface");
			// Note a good idea to access context here
			// as this code is usally executed inside the
			// destructor of the context :
			// ~VulkanContext() -> ~VulkanDevice() -> ~VulkanSwapchain()
			m_Device->GetParentContext()->DestroySurface(m_Surface);
			log::Info("Window Surface destroyed");
		}
	}

	Result<U32, vk::Result> VulkanSwapchain::AcquireNextImage(vk::Semaphore semaphore, vk::Fence fence, U64 timeout)
	{
		U32 imageIndex = 0;
		auto result = m_Device->GetDevice().acquireNextImageKHR(m_Swapchain, timeout, semaphore, fence, &imageIndex);
		return result == vk::Result::eSuccess ? Ok<U32, vk::Result>(imageIndex) : Err<U32, vk::Result>(result);
	}

	vk::Result VulkanSwapchain::PresentImage(U32 index, vk::Semaphore waitSemaphore)
	{
		auto presentInfo = vk::PresentInfoKHR()
							   .setWaitSemaphoreCount(1)
							   .setPWaitSemaphores(&waitSemaphore)
							   .setSwapchainCount(1)
							   .setPSwapchains(&m_Swapchain)
							   .setPImageIndices(&index);

		return m_Device->GetQueue(Present).presentKHR(presentInfo);
	}

	Bool VulkanSwapchain::ChooseSufaceFormat()
	{
		const auto [result, formats] = m_Device->GetPhysicalDevice().getSurfaceFormatsKHR(m_Surface);
		VkCritCall(result);

		if (formats.empty())
		{
			log::Error("No surface formats found");
			return false;
		}

		for (const auto &format : formats)
		{
			if (format.format == vk::Format::eB8G8R8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			{
				m_SwapchainImageFormat = format.format;
				return true;
			}
		}

		m_SwapchainImageFormat = formats[0];
		return true;
	}

	Bool VulkanSwapchain::ChoosePresentMode()
	{
		const auto [result, modes] = m_Device->GetPhysicalDevice().getSurfacePresentModesKHR(m_Surface);
		VkCritCall(result);

		if (modes.empty())
		{
			log::Error("No surface present modes found");
			return false;
		}

		for (const auto &mode : modes)
		{
			if (mode == vk::PresentModeKHR::eMailbox)
			{
				m_PresentMode = mode;
				return true;
			}
		}

		m_PresentMode = vk::PresentModeKHR::eFifo;
		return true;
	}

	Bool VulkanSwapchain::ChooseExtent()
	{
		const auto [result, capabilities] = m_Device->GetPhysicalDevice().getSurfaceCapabilitiesKHR(m_Surface);
		VkCritCall(result);

		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			m_SwapchainExtent = capabilities.currentExtent;
			return true;
		}

		const auto [width, height] = m_Window->GetFramebufferSize();

		m_SwapchainExtent.setWidth(std::clamp(static_cast<U32>(width), capabilities.minImageExtent.width, capabilities.maxImageExtent.width));
		m_SwapchainExtent.setHeight(std::clamp(static_cast<U32>(height), capabilities.minImageExtent.height, capabilities.maxImageExtent.height));

		return true;
	}

	Bool VulkanSwapchain::Recreate()
	{
		Cleanup(true);

		if (!ChooseSufaceFormat())
		{
			log::Error("Failed to choose surface format");
			return false;
		}

		if (!ChoosePresentMode())
		{
			log::Error("Failed to choose present mode");
			return false;
		}

		if (!ChooseExtent())
		{
			log::Error("Failed to choose extent");
			return false;
		}

		const auto [result, capabilities] = m_Device->GetPhysicalDevice().getSurfaceCapabilitiesKHR(m_Surface);
		if (result != vk::Result::eSuccess)
		{
			log::Error("Failed to get surface capabilities: {}", vk::to_string(result));
			return false;
		}

		U32 imageCount = capabilities.minImageCount + 1;

		if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
		{
			imageCount = capabilities.maxImageCount;
		}

		vk::SwapchainCreateInfoKHR createInfo = vk::SwapchainCreateInfoKHR()
													.setSurface(m_Surface)
													.setMinImageCount(imageCount)
													.setImageFormat(m_SwapchainImageFormat.format)
													.setImageColorSpace(m_SwapchainImageFormat.colorSpace)
													.setImageExtent(m_SwapchainExtent)
													.setImageArrayLayers(1)
													.setQueueFamilyIndexCount(0)
													.setPQueueFamilyIndices(nullptr)
													// .setImageUsage(vk::ImageUsageFlagBits::eTransferDst);
													.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

		const U32 queueFamilyIndices[] = {static_cast<U32>(m_Device->GetGraphicsQueueFamilyIndex()), static_cast<U32>(m_Device->GetPresentQueueFamilyIndex())};
		if (m_Device->GetGraphicsQueueFamilyIndex() != m_Device->GetPresentQueueFamilyIndex())
		{
			createInfo.setImageSharingMode(vk::SharingMode::eConcurrent)
				.setQueueFamilyIndexCount(2)
				.setPQueueFamilyIndices(queueFamilyIndices);
		}
		else
		{
			createInfo.setImageSharingMode(vk::SharingMode::eExclusive);
		}

		createInfo.setPreTransform(capabilities.currentTransform)
			.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
			.setPresentMode(m_PresentMode)
			.setClipped(true)
			.setOldSwapchain(m_Swapchain);

		auto [result2, newSwapChain] = m_Device->GetDevice().createSwapchainKHR(createInfo);

		if (result2 != vk::Result::eSuccess)
		{
			if (m_Swapchain != static_cast<vk::SwapchainKHR>(VK_NULL_HANDLE))
			{
				log::Warn("Failed to create new swapchain, using old one");
				newSwapChain = m_Swapchain;
				m_Swapchain = static_cast<vk::SwapchainKHR>(VK_NULL_HANDLE);
			}
			else
			{
				log::Error("Failed to create swapchain");
				return false;
			}
		}

		if (m_Swapchain != static_cast<vk::SwapchainKHR>(VK_NULL_HANDLE))
		{
			m_Device->GetDevice().destroySwapchainKHR(m_Swapchain);
		}

		m_Swapchain = newSwapChain;

		if (!QuerySwapchainImages())
		{
			log::Error("Failed to query swapchain images");
			return false;
		}

		if (!CreateImageViews())
		{
			log::Error("Failed to create image views");
			return false;
		}

		m_IsReady = true;

		return true;
	}

	Bool VulkanSwapchain::QuerySwapchainImages()
	{
		auto [result, images] = m_Device->GetDevice().getSwapchainImagesKHR(m_Swapchain);
		if (result != vk::Result::eSuccess)
		{
			log::Error("Failed to get swapchain images: {}", vk::to_string(result));
			return false;
		}
		m_Images = images;
		return true;
	}

	Bool VulkanSwapchain::CreateImageViews()
	{
		m_ImageViews.resize(m_Images.size());

		for (U32 i = 0; i < m_Images.size(); i++)
		{
			VkImageSubresourceRange subresourceRange = vk::ImageSubresourceRange()
														   .setAspectMask(vk::ImageAspectFlagBits::eColor)
														   .setBaseMipLevel(0)
														   .setLevelCount(1)
														   .setBaseArrayLayer(0)
														   .setLayerCount(1);

			vk::ImageViewCreateInfo createInfo = vk::ImageViewCreateInfo()
													 .setImage(m_Images[i])
													 .setViewType(vk::ImageViewType::e2D)
													 .setFormat(m_SwapchainImageFormat.format)
													 .setComponents(vk::ComponentMapping())
													 .setSubresourceRange(subresourceRange);

			auto [result, imageView] = m_Device->GetDevice().createImageView(createInfo);
			if (result != vk::Result::eSuccess)
			{
				log::Error("Failed to create image view: {}", vk::to_string(result));
				return false;
			}
			m_ImageViews[i] = imageView;

			if (m_ImageViews[i] == static_cast<vk::ImageView>(VK_NULL_HANDLE))
			{
				log::Error("Failed to create image view");
				return false;
			}
		}

		return true;
	}

	List<vk::Framebuffer> VulkanSwapchain::CreateFramebuffers(const vk::RenderPass& renderPass) const {
		List<vk::Framebuffer> framebuffers(m_Images.size());
		for (U32 i = 0; i < m_Images.size(); i++)
		{
			Array<vk::ImageView, 1> attachments = { m_ImageViews[i] };
			vk::FramebufferCreateInfo framebufferInfo = vk::FramebufferCreateInfo()
				.setRenderPass(renderPass)
				.setAttachmentCount(static_cast<U32>(attachments.size()))
				.setPAttachments(attachments.data())
				.setWidth(m_SwapchainExtent.width)
				.setHeight(m_SwapchainExtent.height)
				.setLayers(1);

			auto [result, framebuffer] = m_Device->GetDevice().createFramebuffer(framebufferInfo);
			if (result != vk::Result::eSuccess)
			{
				log::Error("Failed to create framebuffer: {}", vk::to_string(result));
				return {};
			}

			framebuffers[i] = framebuffer;
			if (framebuffers[i] == static_cast<vk::Framebuffer>(VK_NULL_HANDLE)) {
				log::Fatal("Failed to create framebuffer from swapchain image view: {}", i);
			}
		}

		return framebuffers;
	}

	void VulkanSwapchain::Cleanup(Bool forRecreate)
	{
		if (!m_IsReady)
		{
			return;
		}

		m_Device->WaitIdle();
		for (auto &imageView : m_ImageViews)
		{
			m_Device->GetDevice().destroyImageView(imageView);
		}
		m_ImageViews.clear();
		if (!forRecreate)
			m_Device->GetDevice().destroySwapchainKHR(m_Swapchain);
		m_IsReady = false;
	}

	

}