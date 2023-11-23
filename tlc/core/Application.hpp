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

		inline static Application* Get() { if (!s_Instance) s_Instance = CreateScope<Application>(); return s_Instance.get(); }
		inline static void Shutdown() { s_Instance.reset(); }
	
	private:
		bool m_Running = true;
		bool m_Paused = false;
		bool m_Minimized = false;
		Window* m_Window = nullptr;
		VulkanContext* m_VulkanContext = nullptr;
		VulkanDevice* m_VulkanDevice = nullptr;

		Renderer* m_Renderer = nullptr;

		Ref<VulkanSwapchain> m_VulkanSwapchain = nullptr;
		
		Ref<VulkanGraphicsPipeline> m_Pipeline;
		Ref<VulkanBuffer> m_VertexBuffer;


		F32 m_DeltaTime = 0.0f;

		static Scope<Application> s_Instance;
	};
}