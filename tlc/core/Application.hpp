#pragma once
#include "core/Core.hpp"
#include "core/Window.hpp"
#include "vulkan/VulkanContext.hpp"
#include "vulkan/VulkanSwapchain.hpp"

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

		inline static Application* Get() { if (!s_Instance) s_Instance = CreateScope<Application>(); return s_Instance.get(); }
		inline static void Shutdown() { s_Instance.reset(); }
	
	private:
		bool m_Running = true;
		bool m_Paused = false;
		bool m_Minimized = false;
		Window* m_Window = nullptr;
		VulkanContext* m_VulkanContext = nullptr;
		VulkanDevice* m_VulkanDevice = nullptr;
		VulkanSwapchain* m_VulkanSwapchain = nullptr;

		static Scope<Application> s_Instance;
	};
}