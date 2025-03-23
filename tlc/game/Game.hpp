#pragma once

#include "core/Application.hpp"

namespace tlc 
{
	class VulkanManager;

	class GameApplication : public Application
	{
	public:
		GameApplication() = default;
		virtual ~GameApplication() = default;

		virtual void OnLoad() override;
		virtual void OnUnload() override;
		virtual void OnStart() override;
		virtual void OnUpdate() override;
		virtual void OnEnd() override;


		inline static Application* Create() { s_Instance = CreateScope<GameApplication>(); return s_Instance.get(); }

	private:
		void RegisterAssets();
		void RegisterServices();
		void RenderEngineFrame();

	private:
		F32 m_LastFrameTime = 0.0f;
	};

}