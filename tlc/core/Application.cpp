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

		m_Renderer = Renderer::Get(m_VulkanDevice, m_VulkanSwapchain.get());
	}

	Application::~Application()
	{
		log::Debug("Shutting down application");

		m_VulkanDevice->WaitIdle();
		
		m_VertexBuffer.reset();
		m_Pipeline.reset();

		Renderer::Shutdown();

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
		log::Info("Pipeline is ready : {}", m_Pipeline->IsReady());

		m_VertexBuffer = m_VulkanDevice->CreateBuffer();
		m_VertexBuffer->SetUsageFlags(vk::BufferUsageFlagBits::eVertexBuffer);

		m_VertexBuffer->Resize(sizeof(VulkanVertex) * 3);

		

		vertices[0].position = glm::vec4(0.0f, -0.5f, 0.0f, 1.0f);
		vertices[0].color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		vertices[1].position = glm::vec4(0.5f, 0.5f, 0.0f, 1.0f);
		vertices[1].color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
		vertices[2].position = glm::vec4(-0.5f, 0.5f, 0.0f, 1.0f);
		vertices[2].color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		m_VertexBuffer->SetData(vertices, sizeof(vertices));
	}

	void Application::Run()
	{
		auto prevWindowSize = m_Window->GetSize();
		auto prevTime = glfwGetTime();

		I32 fps = 0;
		F32 fpsTimer = 0.0f;

		m_Renderer->SetClearColor(0.2f, 0.21f, 0.22f, 1.0f);

		while (m_Running)
		{
			auto currentTime = glfwGetTime();
			m_DeltaTime = static_cast<F32>(currentTime - prevTime);
			prevTime = currentTime;

			fpsTimer += m_DeltaTime;
			fps++;
			if (fpsTimer >= 1.0f)
			{
				m_Window->SetTitle(std::format("TLC [Delta time: {}, FPS: {}]", m_DeltaTime, fps));
				fps = 0;
				fpsTimer = 0.0f;
			}

			//m_Window->SetTitle(std::format("TLC [Delta time: {}, FPS: {}]", m_DeltaTime, 1.0f / m_DeltaTime));

			if (!m_Minimized)
			{
				const auto currentWindowSize = m_Window->GetSize();
				if ( currentWindowSize.x != prevWindowSize.x || currentWindowSize.y != prevWindowSize.y )
				{
					m_VulkanDevice->WaitIdle();
					m_VulkanSwapchain->Recreate();
					prevWindowSize = currentWindowSize;
				}

				m_Renderer->AcquireNextImage();

				m_Renderer->BeginFrame();

				m_Renderer->BeginDefaultRenderPass();

				// m_CommandBuffer->BindPipeline(m_Pipeline->GetPipeline());
				m_Renderer->SetPipeline(m_Pipeline.get());
				
				m_Renderer->SetViewport(0.0f, 0.0f, static_cast<F32>(m_VulkanSwapchain->GetExtent().width), static_cast<F32>(m_VulkanSwapchain->GetExtent().height));

				m_Renderer->SetScissor(0, 0, m_VulkanSwapchain->GetExtent().width, m_VulkanSwapchain->GetExtent().height);

				m_Renderer->GetCommandBuffer()->BindVertexBuffer(m_VertexBuffer->GetBuffer(), 0);

				m_Renderer->DrawRaw(3, 1);
				
				m_Renderer->EndRenderPass();
				m_Renderer->EndFrame();

				
				m_Renderer->PresentFrame();

				vertices[0].position = glm::vec4(sinf(static_cast<F32>(currentTime)) * 0.5f, -0.5f, 0.0f, 1.0f);
				vertices[1].position = glm::vec4(0.5f, sinf(static_cast<F32>(currentTime + 115.0f)) * 0.5f, 0.0f, 1.0f);
				vertices[2].position = glm::vec4(-0.5f, sinf(static_cast<F32>(currentTime + 230.0f)) * 0.5f, 0.0f, 1.0f);
				m_VertexBuffer->SetData(vertices, sizeof(vertices));

			}



			m_Window->Update();
			m_Window->SetFullscreen(glfwGetKey(m_Window->GetHandle(), GLFW_KEY_F) == GLFW_PRESS);

		}
	}

}