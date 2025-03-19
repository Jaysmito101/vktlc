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

		log::Info("Application started");

		Window::Get(); // Setup window
		Services::Setup();

		EventManager<EventType::WindowClose>::Get()->Subscribe([this]() -> void {
			m_Running = false;
			});

	}

	Application::~Application()
	{
		m_Scenes.clear();

		log::Debug("Shutting down application");

		Services::Shutdown();
		Window::Shutdown();
		log::Info("Application shutdown");
	}

	void Application::Run()
	{
		OnLoad();
		m_HasLoaded = true;
		m_LastFrameTime = static_cast<F32>(glfwGetTime());
		OnStart();

		if (m_CurrentScene != nullptr) m_CurrentScene->Start();

		auto window = Window::Get();

		try 
		{
			while (m_Running)
			{
				m_CurentFrameTime = static_cast<F32>(glfwGetTime());
				m_DeltaTime = static_cast<F32>(m_CurentFrameTime - m_LastFrameTime);
				m_LastFrameTime = m_CurentFrameTime;
				m_FramerateTimer += m_DeltaTime;
				m_CurrentFramerateCounter += 1;
				if (m_FramerateTimer >= 1.0f)
				{
					m_CurrentFramerate = m_CurrentFramerateCounter;
					m_FramerateTimer = 0.0f;
				}

				
				if(m_NextSceneOnLoading) PollForSceneChange();
				window->Update();

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