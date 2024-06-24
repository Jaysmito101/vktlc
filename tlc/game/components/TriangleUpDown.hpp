#pragma once

#include "engine/ecs/ECS.hpp"
#include "rendering/Renderer.hpp"

namespace tlc {

	class TriangleUpDown : public IComponent
	{
	public:
		TriangleUpDown() : IComponent() {}
		~TriangleUpDown() {}

		void Resize(U32 width, U32 height);

	protected:
		void OnStart() override;
		void OnUpdate() override;
		void OnEnd() override;

	private:
		VulkanGraphicsPipelineSettings m_PipelineSettings = {};

		Ref<VulkanGraphicsPipeline> m_Pipeline;
		Ref<VulkanBuffer> m_VertexBuffer;
		VulkanVertex vertices[3];
	};

}
