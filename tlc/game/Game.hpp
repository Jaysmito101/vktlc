#pragma once

#include "core/Application.hpp"

namespace tlc 
{

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

		virtual void OnResize(U32 width, U32 height) override;


		inline static Application* Create() { s_Instance = CreateScope<GameApplication>(); return s_Instance.get(); }

	private:
		Ref<VulkanGraphicsPipeline> m_Pipeline;
		Ref<VulkanBuffer> m_VertexBuffer;
		VulkanVertex vertices[3];


		I32 fps = 0;
		F32 fpsTimer = 0.0f;
	};

}