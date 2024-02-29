#pragma once
#include "core/Core.hpp"
#include "core/Window.hpp"
#include "vulkan/VulkanContext.hpp"
#include "vulkan/VulkanSwapchain.hpp"
#include "rendering/Renderer.hpp"

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

		virtual void OnLoad() = 0;
		virtual void OnUnload() = 0;
		virtual void OnStart() = 0;
		virtual void OnUpdate() = 0;
		virtual void OnEnd() = 0;

		virtual void OnResize(U32 width, U32 height) = 0;

		virtual void OnPause() {}; // Unused
		virtual void OnResume() {}; // Unused



	protected:
		
		static Scope<Application> s_Instance;

	
		Window* m_Window = nullptr;
		VulkanContext* m_VulkanContext = nullptr;
		VulkanDevice* m_VulkanDevice = nullptr;
		Renderer* m_Renderer = nullptr;
		Ref<VulkanSwapchain> m_VulkanSwapchain = nullptr;		

	private:
		bool m_Running = true;
		bool m_Paused = false;
		bool m_Minimized = false;
		F32 m_DeltaTime = 0.0f;
		F32 m_LastFrameTime = 0.0f;
		F32 m_CurentFrameTime = 0.0f;
	};
}
