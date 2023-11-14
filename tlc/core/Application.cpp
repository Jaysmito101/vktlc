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
		while (m_Running)
		{
			if (!m_Minimized)
			{



			}

			m_Window->Update();
			m_Window->SetFullscreen(glfwGetKey(m_Window->GetHandle(), GLFW_KEY_F) == GLFW_PRESS);

		}
	}

}