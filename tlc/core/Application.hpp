#pragma once
#include "core/Core.hpp"
#include "core/Window.hpp"
#include "vulkan/VulkanContext.hpp"

namespace tlc 
{
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void SetupVulkan();

		void Run();

		inline static Application* Get() { if (!s_Instance) s_Instance = CreateScope<Application>(); return s_Instance.get(); }
		inline static void Shutdown() { s_Instance.reset(); }
	
	private:
		bool m_Running = true;
		Window* m_Window = nullptr;
		VulkanContext* m_VulkanContext = nullptr;
			 
		static Scope<Application> s_Instance;
	};
}