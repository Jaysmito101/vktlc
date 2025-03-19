#pragma once
#include "core/Core.hpp"
#include "core/Window.hpp"
#include "vulkanapi/VulkanContext.hpp"
#include "vulkanapi/VulkanSwapchain.hpp"

#include "engine/Scene.hpp"

namespace tlc 
{
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void SetupVulkan();
		void Run();
		
		inline Bool IsRunning() const { return m_Running; }
		inline Bool IsPaused() const { return m_Paused; }
		inline Bool IsMinimized() const { return m_Minimized; }
		inline F32 GetDeltaTime() const { return m_DeltaTime; }
		inline F32 GetLastFrameTime() const { return m_LastFrameTime; }
		inline F32 GetCurrentFrameTime() const { return m_CurentFrameTime; }
		inline I32 GetCurrentFramerate() const { return m_CurrentFramerate; }

		inline static Raw<Application> Get() { TLC_ASSERT(s_Instance != nullptr , "Instance cannot be null!"); return s_Instance.get(); }
		inline static void Shutdown() { s_Instance.reset(); }
		inline Raw<Scene> GetCurrentScene() { return m_CurrentScene; }

		virtual void OnLoad() = 0;
		virtual void OnUnload() = 0;
		virtual void OnStart() = 0;
		virtual void OnUpdate() = 0;
		virtual void OnEnd() = 0;


		template <typename T>
		void RegisterScene(const String& name)
		{
			TLC_ASSERT(!m_HasLoaded, "Cannot Register scenes after loading!");
			TLC_ASSERT(m_Scenes.find(name) == m_Scenes.end(), "Scene already exists!");
			m_Scenes[name] = CreateScope<T>();
			m_Scenes[name]->SetName(name);
		}

		inline void ChangeScene(const String& name) { m_SceneChangeRequest = std::make_pair(name, false); }
		void ChangeSceneAsync(const String& name) { m_SceneChangeRequest = std::make_pair(name, true); }

	private:
		void PollForSceneChange();


		void ChangeSceneI(const String& name);
		void ChangeSceneAsyncI(const String& name);

	protected:
		
		static Scope<Application> s_Instance;

	private:
		Bool m_Running = true;
		Bool m_Paused = false;
		Bool m_Minimized = false;
		Bool m_HasLoaded = false;
		F32 m_DeltaTime = 0.0f;
		F32 m_LastFrameTime = 0.0f;
		F32 m_CurentFrameTime = 0.0f;
		I32 m_CurrentFramerate = 0;
		I32 m_CurrentFramerateCounter = 0;
		F32 m_FramerateTimer = 0.0f;

		Map<String, Scope<Scene>> m_Scenes;
		Raw<Scene> m_CurrentScene = nullptr;
		Raw<Scene> m_NextSceneOnLoading = nullptr;


		std::optional<std::pair<String, Bool>> m_SceneChangeRequest = std::nullopt;
	};
}
