#pragma once
#include "core/Core.hpp"
#include "core/Window.hpp"
#include "vulkan/VulkanContext.hpp"
#include "vulkan/VulkanSwapchain.hpp"
#include "rendering/Renderer.hpp"

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

		inline static Application* Get() { TLC_ASSERT(s_Instance != nullptr , "Instance cannot be null!"); return s_Instance.get(); }
		inline static void Shutdown() { s_Instance.reset(); }

		// TODO: Maybe remove these
		inline VulkanDevice* GetVulkanDevice() { return m_VulkanDevice; }
		inline VulkanSwapchain* GetVulkanSwapchain() { return m_VulkanSwapchain.get(); }
		inline Renderer* GetRenderer() { return m_Renderer; }
		inline Scene* GetCurrentScene() { return m_CurrentScene; }

		virtual void OnLoad() = 0;
		virtual void OnUnload() = 0;
		virtual void OnStart() = 0;
		virtual void OnUpdate() = 0;
		virtual void OnEnd() = 0;
		virtual void OnResize(U32 width, U32 height) = 0;


		template <typename T>
		void RegisterScene(const String& name)
		{
			TLC_ASSERT(!m_HasLoaded, "Cannot Register scenes after loading!");
			TLC_ASSERT(m_Scenes.find(name) == m_Scenes.end(), "Scene already exists!");
			m_Scenes[name] = CreateScope<T>();
			m_Scenes[name]->SetName(name);
		}

		void ChangeScene(const String& name);
		void ChangeSceneAsync(const String& name);

	private:
		void PollForSceneChange();

	protected:
		
		static Scope<Application> s_Instance;

	
		Window* m_Window = nullptr;
		VulkanContext* m_VulkanContext = nullptr;
		VulkanDevice* m_VulkanDevice = nullptr;
		Renderer* m_Renderer = nullptr;
		Ref<VulkanSwapchain> m_VulkanSwapchain = nullptr;		

	private:
		Bool m_Running = true;
		Bool m_Paused = false;
		Bool m_Minimized = false;
		Bool m_HasLoaded = false;
		F32 m_DeltaTime = 0.0f;
		F32 m_LastFrameTime = 0.0f;
		F32 m_CurentFrameTime = 0.0f;

		Map<String, Scope<Scene>> m_Scenes;
		Scene* m_CurrentScene = nullptr;
		Scene* m_NextSceneOnLoading = nullptr;
	};
}
