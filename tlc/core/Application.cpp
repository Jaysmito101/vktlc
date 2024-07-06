#include "core/Application.hpp"
#include "services/Services.hpp"

// TODO: Make input system!
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

		Services::Setup();

		EventManager<EventType::WindowClose>::Get()->Subscribe([this]() -> void {
			m_Running = false;
			});

		// TODO: Clean this up!
		EventManager<EventType::WindowFramebufferSize, I32, I32>::Get()->Subscribe([this](I32 width, I32 height) -> void {
			m_Minimized = (width == 0 || height == 0);
			static Pair<I32, I32> prevWindowSize = MakePair(0, 0);
			if ((width != prevWindowSize.x || height != prevWindowSize.y) && (width > 0 && height > 0))
			{
				m_VulkanDevice->WaitIdle();
				m_VulkanSwapchain->Recreate();
				// TODO: Recreate the pipeline here
				prevWindowSize = MakePair(width, height);
			}
			OnResize(width, height);
			});


		SetupVulkan();

		m_Renderer = Renderer::Get(m_VulkanDevice, m_VulkanSwapchain.get());

	}

	Application::~Application()
	{
		m_Scenes.clear();

		log::Debug("Shutting down application");

		Services::Shutdown();

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
		OnLoad();

		m_HasLoaded = true;

	
		m_LastFrameTime = static_cast<F32>(glfwGetTime());
		m_Renderer->SetClearColor(0.2f, 0.21f, 0.22f, 1.0f);

		OnStart();


		if (m_CurrentScene != nullptr) m_CurrentScene->Start();

		try 
		{
			while (m_Running)
			{
				m_CurentFrameTime = static_cast<F32>(glfwGetTime());
				m_DeltaTime = static_cast<F32>(m_CurentFrameTime - m_LastFrameTime);
				m_LastFrameTime = m_CurentFrameTime;
				
				if(m_NextSceneOnLoading) PollForSceneChange();
				m_Window->Update();

				if (m_Minimized) continue;

				if(m_CurrentScene != nullptr && !m_CurrentScene->IsPaused()) m_CurrentScene->Update();
				OnUpdate();

				if (m_SceneChangeRequest.has_value()) {
					if (m_SceneChangeRequest.value().second)
						ChangeSceneAsyncI(m_SceneChangeRequest.value().first);
					else
						ChangeSceneI(m_SceneChangeRequest.value().first);
					m_SceneChangeRequest.reset();
				}
			}
		}
		catch (const std::exception& e)
		{
			log::Error("Exception: {}", e.what());
		}

		if (m_CurrentScene != nullptr) m_CurrentScene->End();
		OnEnd();

		if (m_CurrentScene != nullptr) m_CurrentScene->Unload();

		m_VulkanDevice->WaitIdle();
		OnUnload();

	}

	void Application::ChangeSceneI(const String& name)
	{
		TLC_ASSERT(m_Scenes.find(name) != m_Scenes.end(), "Scene not found");

		if (m_CurrentScene != nullptr)
		{
			m_CurrentScene->End();
			m_CurrentScene->Unload();
		}
		
		m_CurrentScene = m_Scenes[name].get();
		m_CurrentScene->Load(false);
		m_CurrentScene->Start();
		Services::PushSceneChangeEvent();
	}

	void Application::ChangeSceneAsyncI(const String& name)
	{
		TLC_ASSERT(m_Scenes.find(name) != m_Scenes.end(), "Scene not found");

		if (m_NextSceneOnLoading != nullptr)
		{
			log::Error("Cannot load another scene while one is already loading");
			return;
		}

		if (m_CurrentScene && m_CurrentScene->GetName() == name)
		{
			log::Error("Cannot load the same scene again async, use ChangeScene instead");
			return;
		}


		// First load the new scene async
		m_NextSceneOnLoading = m_Scenes[name].get();

		std::thread([this]() -> void {
			m_NextSceneOnLoading->Load(true);
			}).detach();
	}

	void Application::PollForSceneChange()
	{
		if (m_NextSceneOnLoading != nullptr && m_NextSceneOnLoading->HasLoaded())
		{
			if (m_CurrentScene != nullptr)
			{
				m_CurrentScene->End();
				m_CurrentScene->Unload();
			}

			m_CurrentScene = m_NextSceneOnLoading;
			m_NextSceneOnLoading = nullptr;

			m_CurrentScene->Start();
			Services::PushSceneChangeEvent();
		}
	}



}