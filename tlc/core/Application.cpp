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

		m_Window = Window::Get();
		log::Info("Application started");

		EventManager<EventType::WindowClose>::Get()->Subscribe([this]() -> void {
			m_Running = false;
			});

		EventManager<EventType::WindowFramebufferSize, I32, I32>::Get()->Subscribe([this](I32 width, I32 height) -> void {
			m_Minimized = (width == 0 || height == 0);
			this->OnResize(width, height);
			});


		SetupVulkan();

		m_Renderer = Renderer::Get(m_VulkanDevice, m_VulkanSwapchain.get());

	}

	Application::~Application()
	{

		log::Debug("Shutting down application");

		m_VulkanDevice->WaitIdle();

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

	}

	void Application::Run()
	{
		m_VulkanDevice->WaitIdle();
		this->OnLoad();
	
		auto prevWindowSize = m_Window->GetSize();
		m_LastFrameTime = static_cast<F32>(glfwGetTime());

		m_Renderer->SetClearColor(0.2f, 0.21f, 0.22f, 1.0f);
		this->OnStart();

		try 
		{
			while (m_Running)
			{
				m_CurentFrameTime = static_cast<F32>(glfwGetTime());
				m_DeltaTime = static_cast<F32>(m_CurentFrameTime - m_LastFrameTime);
				m_LastFrameTime = m_CurentFrameTime;

				this->OnUpdate();


				m_Window->Update();

			}
		}
		catch (const std::exception& e)
		{
			log::Error("Exception: {}", e.what());
		}

		this->OnEnd();


		m_VulkanDevice->WaitIdle();
		this->OnUnload();

	}

}