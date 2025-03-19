#include "game/components/TriangleUpDown.hpp"
#include "core/Application.hpp"


#include "services/CacheManager.hpp"


// TODO: Make input system!
#include <GLFW/glfw3.h>

namespace tlc 
{
	void TriangleUpDown::OnStart() 
	{
		vertices[0].position = glm::vec4(0.25f, -0.5f, 0.0f, 1.0f);
		vertices[0].color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
		vertices[1].position = glm::vec4(0.5f, 0.5f, 0.0f, 1.0f);
		vertices[1].color = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
		vertices[2].position = glm::vec4(-0.5f, 0.5f, 0.0f, 1.0f);
		vertices[2].color = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);

		const auto swapchain = Application::Get()->GetVulkanSwapchain();
		const auto device = Application::Get()->GetVulkanDevice();

		auto cacheService = Services::Get<CacheManager>();
		auto vertShaderModule = device->CreateShaderModule(cacheService->GetCacheDataTyped<U32>("shaders/vert.glsl"));
		auto fragShaderModule = device->CreateShaderModule(cacheService->GetCacheDataTyped<U32>("shaders/frag.glsl"));


		m_PipelineSettings = VulkanGraphicsPipelineSettings()
			.SetExtent(swapchain->GetExtent())
			.SetVertexShaderModule(vertShaderModule)
			.SetFragmentShaderModule(fragShaderModule);

		m_Pipeline = swapchain->GetFramebuffers()[0]->CreateGraphicsPipeline(m_PipelineSettings);

		log::Info("Pipeline is ready : {}", m_Pipeline->IsReady());

		m_VertexBuffer = device->CreateBuffer();
		m_VertexBuffer->SetUsageFlags(vk::BufferUsageFlagBits::eVertexBuffer);
		m_VertexBuffer->Resize(sizeof(VulkanVertex) * 3);
		m_VertexBuffer->SetData(vertices, sizeof(vertices));

		log::Info("TriangleUpDown::OnStart() called");
	}

	void TriangleUpDown::OnUpdate() 
	{
		const auto renderer = Application::Get()->GetRenderer();
		const auto swapchain = Application::Get()->GetVulkanSwapchain();

		renderer->AcquireNextImage();
		renderer->BeginFrame();
		renderer->BeginDefaultRenderPass();
		renderer->SetPipeline(m_Pipeline.get());
		renderer->SetViewport(0.0f, 0.0f, static_cast<F32>(swapchain->GetExtent().width), static_cast<F32>(swapchain->GetExtent().height));
		renderer->SetScissor(0, 0, swapchain->GetExtent().width, swapchain->GetExtent().height);
		renderer->GetCommandBuffer()->BindVertexBuffer(m_VertexBuffer->GetBuffer(), 0);
		renderer->DrawRaw(3, 1);
		renderer->EndRenderPass();
		renderer->EndFrame();
		renderer->PresentFrame();

		vertices[0].position = glm::vec4(0.5f * fabsf(sinf((F32)glfwGetTime()) + 0.05f), -0.5f, 0.0f, 1.0f);
		m_VertexBuffer->SetData(vertices, sizeof(F32));

		//log::Info("TriangleUpDown::OnUpdate() called");
	}

	void TriangleUpDown::OnEnd()
	{
		m_VertexBuffer.reset();
		m_Pipeline.reset();

		log::Info("TriangleUpDown::OnEnd() called");
	}

	void TriangleUpDown::Resize(U32 width, U32 height)
	{
		(void)width; (void)height;

		const auto swapchain = Application::Get()->GetVulkanSwapchain();

		m_PipelineSettings.SetExtent(swapchain->GetExtent());
		m_Pipeline = swapchain->GetFramebuffers()[0]->CreateGraphicsPipeline(m_PipelineSettings);

		log::Info("TriangleUpDown::Resize() called");
	}

}