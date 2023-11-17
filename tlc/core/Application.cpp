#include "core/Application.hpp"

#include <GLFW/glfw3.h>

namespace tlc
{

	Scope<Application> Application::s_Instance = nullptr;

	Application::Application()
	{
		if (s_Instance != nullptr)
		{
			log::Fatal("Application already exists!");
		}

		log::Debug("Starting up application");
		log::AttachFile(utils::GetExecutableDirectory() + "/log.txt");

		m_Window = Window::Get();
		log::Info("Application started");

		EventManager<EventType::WindowClose>::Get()->Subscribe([this]() -> void {
			m_Running = false;
			});

		EventManager<EventType::WindowFramebufferSize, I32, I32>::Get()->Subscribe ([this](I32 width, I32 height) -> void {
			m_Minimized = (width == 0 || height == 0);
		});

		SetupVulkan();
	}

	Application::~Application()
	{
		log::Debug("Shutting down application");

		m_CommandBuffer.reset();

		m_VulkanDevice->DestroyVkSemaphore (m_ImageAvailableSemaphore);
		m_VulkanDevice->DestroyVkSemaphore (m_RenderFinishedSemaphore);
		m_VulkanDevice->DestroyVkFence (m_InFlightFence);

		 m_Pipeline.reset();

		m_VulkanSwapchain.reset();
		Window::Shutdown();
		VulkanContext::Shutdown();
		log::Info("Application shutdown");
	}

	void Application::SetupVulkan()
	{
		m_VulkanContext = VulkanContext::Get();

		auto physicalDevice = m_VulkanContext->PickPhysicalDevice();
		log::Info("Picked physical device: {}", physicalDevice.getProperties().deviceName.data());
		m_VulkanDevice = m_VulkanContext->CreateDevice(physicalDevice);

		log::Debug("Creating surface");
		m_VulkanContext->CreateSurface(m_Window);
		
		log::Debug("Creating swapchain");
		m_VulkanSwapchain = m_VulkanDevice->CreateSwapchain(m_Window);
		if (m_VulkanSwapchain == nullptr)
		{
			log::Fatal("Failed to create swapchain");
		}
		log::Info("Created swapchain");

		auto vertModule = m_VulkanDevice->CreateShaderModule(utils::ReadBinaryFie("vert.spv"));
		auto fragModule = m_VulkanDevice->CreateShaderModule(utils::ReadBinaryFie("frag.spv"));

		auto settings = VulkanGraphicsPipelineSettings()
			.SetExtent(m_VulkanSwapchain->GetExtent())
			.SetVertexShaderModule(vertModule)
			.SetFragmentShaderModule(fragModule);

		m_Pipeline = m_VulkanSwapchain->GetFramebuffers()[0]->CreateGraphicsPipeline(settings);
		log::Info("Pipeline is ready ? : {}", m_Pipeline->IsReady());

		m_ImageAvailableSemaphore = m_VulkanDevice->CreateVkSemaphore();
		m_RenderFinishedSemaphore = m_VulkanDevice->CreateVkSemaphore();
		m_CommandBuffer = m_VulkanDevice->CreateCommandBuffer(Graphics);
		m_InFlightFence = m_VulkanDevice->CreateVkFence( vk::FenceCreateFlagBits::eSignaled );


	}

	void Application::Run()
	{
		auto prevWindowSize = m_Window->GetSize();
		while (m_Running)
		{
			if (!m_Minimized)
			{
				const auto currentWindowSize = m_Window->GetSize();
				if ( currentWindowSize.x != prevWindowSize.x || currentWindowSize.y != prevWindowSize.y )
				{
					m_VulkanDevice->WaitIdle();
					m_VulkanSwapchain->Recreate();



					prevWindowSize = currentWindowSize;
				}

				VkCall( m_VulkanDevice->GetDevice().waitForFences({ m_InFlightFence }, true, UINT64_MAX) );
				m_VulkanDevice->GetDevice().resetFences({m_InFlightFence});

				auto imageIndex = m_VulkanSwapchain->AcquireNextImage(m_ImageAvailableSemaphore);

				m_CommandBuffer->Reset();
				m_CommandBuffer->Begin();

				auto clearColor = vk::ClearColorValue(std::array<float, 4>{0.2f, 0.2f, 0.2f, 1.0f});
				m_CommandBuffer->BeginRenderPass(m_VulkanSwapchain->GetRenderPass(), m_VulkanSwapchain->GetFramebuffers()[imageIndex]->GetFramebuffer(), m_VulkanSwapchain->GetExtent(), { clearColor });

				m_CommandBuffer->BindPipeline(m_Pipeline->GetPipeline());

				auto viewport = vk::Viewport()
					.setX(0.0f)
					.setY(0.0f)
					.setHeight(static_cast<F32>(m_VulkanSwapchain->GetExtent().height))
					.setWidth(static_cast<F32>(m_VulkanSwapchain->GetExtent().width))
					.setMinDepth(0.0f)
					.setMaxDepth(0.0f);
				m_CommandBuffer->SetViewport(viewport);

				auto scissor = vk::Rect2D()
					.setOffset({0, 0})
					.setExtent(m_VulkanSwapchain->GetExtent());
				m_CommandBuffer->SetScissor(scissor);

				m_CommandBuffer->Draw(3, 1);
				
				m_CommandBuffer->EndRenderPass();
				m_CommandBuffer->End();

				m_CommandBuffer->Submit({ m_ImageAvailableSemaphore }, { m_RenderFinishedSemaphore }, m_InFlightFence);

				//log::Info ("Acquired image index: {}", imageIndex);

				// m_CommandBuffer->Reset();
				
				m_VulkanSwapchain->PresentImage(imageIndex, m_RenderFinishedSemaphore);
			}



			m_Window->Update();
			m_Window->SetFullscreen(glfwGetKey(m_Window->GetHandle(), GLFW_KEY_F) == GLFW_PRESS);

		}
	}

}