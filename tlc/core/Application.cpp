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

		EventManager::Get()->Subscribe("WindowClose", [this](void*) -> bool {
			log::Info("Window close event recieved");
			m_Running = false;
			return true;
		});

		m_VulkanContext = VulkanContext::Get();

	}

	Application::~Application()
	{
		log::Debug("Shutting down application");
		Window::Shutdown();
		VulkanContext::Shutdown();
		log::Info("Application shutdown");
	}

	void Application::Run()
	{
		while (m_Running)
		{
			m_Window->Update();

			m_Window->SetFullscreen(glfwGetKey(m_Window->GetHandle(), GLFW_KEY_F) == GLFW_PRESS);

		}
	}

}