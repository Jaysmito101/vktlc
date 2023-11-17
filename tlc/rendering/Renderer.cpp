#include "rendering/Renderer.hpp"
#include "vulkan/VulkanSwapchain.hpp"


namespace tlc
{

	Scope<Renderer> Renderer::s_Instance = nullptr;




	Renderer::Renderer(VulkanDevice* device, VulkanSwapchain* swapchain)
	{
		m_Device = device;
		m_Swapchain = swapchain;

		m_IsReady = CreateVulkanObjects();

		if (!m_IsReady)
		{
			log::Error("Failed to create Vulkan objects for Renderer");
		}
	}

	Renderer::~Renderer()
	{
		Cleanup();
	}

	void Renderer::AcquireNextImage()
	{
		VkCall(m_Device->GetDevice().waitForFences({ m_InFlightFence }, true, UINT64_MAX));
		m_Device->GetDevice().resetFences({ m_InFlightFence });
		m_SwapchainImageIndex = m_Swapchain->AcquireNextImage(m_ImageAvailableSemaphore);
	}

	void Renderer::BeginFrame()
	{
		m_CommandBuffer->Reset();
		m_CommandBuffer->Begin();
	}

	void Renderer::EndFrame()
	{
		m_CommandBuffer->End();

		m_CommandBuffer->Submit({m_ImageAvailableSemaphore}, {m_RenderFinishedSemaphore}, m_InFlightFence);
	}

	void Renderer::PresentFrame()
	{
		m_Swapchain->PresentImage(m_SwapchainImageIndex, m_RenderFinishedSemaphore);
	}

	void Renderer::BeginDefaultRenderPass()
	{
		m_CommandBuffer->BeginRenderPass(m_Swapchain->GetRenderPass(), m_Swapchain->GetFramebuffers()[m_SwapchainImageIndex]->GetFramebuffer(), m_Swapchain->GetExtent(), { m_ClearColor });
	}

	void Renderer::EndRenderPass()
	{
		m_CommandBuffer->EndRenderPass();
	}

	void Renderer::SetPipeline(VulkanGraphicsPipeline* pipeline)
	{
		m_CommandBuffer->BindPipeline(pipeline->GetPipeline());
	}

	void Renderer::SetViewport(F32 x, F32 y, F32 width, F32 height, F32 minDepth, F32 maxDepth)
	{
		auto viewport = vk::Viewport()
			.setX(x)
			.setY(y)
			.setWidth(width)
			.setHeight(height)
			.setMinDepth(minDepth)
			.setMaxDepth(maxDepth);

		m_CommandBuffer->SetViewport(viewport);
	}

	void Renderer::SetScissor(I32 x, I32 y, U32 width, U32 height)
	{
		auto scissor = vk::Rect2D()
			.setOffset({ x, y })
			.setExtent({ width, height });

		m_CommandBuffer->SetScissor(scissor);
	}

	void Renderer::DrawRaw(U32 vertexCount, U32 instanceCount, U32 firstVertex, U32 firstInstance)
	{
		m_CommandBuffer->Draw(vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void Renderer::Cleanup()
	{
		if (!m_IsReady)
		{
			return;
		}

		m_Device->WaitIdle();

		m_CommandBuffer.reset();

		m_Device->DestroyVkSemaphore(m_ImageAvailableSemaphore);
		m_Device->DestroyVkSemaphore(m_RenderFinishedSemaphore);
		m_Device->DestroyVkFence(m_InFlightFence);
	}

	Bool Renderer::CreateVulkanObjects()
	{

		m_CommandBuffer = m_Device->CreateCommandBuffer(Graphics);

		m_ImageAvailableSemaphore = m_Device->CreateVkSemaphore();
		m_RenderFinishedSemaphore = m_Device->CreateVkSemaphore();

		m_InFlightFence = m_Device->CreateVkFence(vk::FenceCreateFlagBits::eSignaled);

		return true;
	}




}