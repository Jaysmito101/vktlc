#include "game/scenes/MainScene.hpp"
#include "core/Application.hpp"



// TODO: Make input system!
#include <GLFW/glfw3.h>

namespace tlc
{
	void MainScene::OnLoad(Bool isAsync)
	{
		(void) isAsync;
		
		log::Info("Loading Scene: {}", GetName());
		std::this_thread::sleep_for(std::chrono::seconds(1));

		vertices[0].position = glm::vec4(0.25f, -0.5f, 0.0f, 1.0f);
		vertices[0].color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		vertices[1].position = glm::vec4(0.5f, 0.5f, 0.0f, 1.0f);
		vertices[1].color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
		vertices[2].position = glm::vec4(-0.5f, 0.5f, 0.0f, 1.0f);
		vertices[2].color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	}

	void MainScene::OnUnload()
	{
		
	}

	void MainScene::OnStart()
	{
		const auto swapchain = Application::Get()->GetVulkanSwapchain();
		const auto device = Application::Get()->GetVulkanDevice();

		auto vertShaderModule = device->CreateShaderModule(utils::ReadBinaryFie("vert.spv"));
		auto fragShaderModule = device->CreateShaderModule(utils::ReadBinaryFie("frag.spv"));

		m_PipelineSettings = VulkanGraphicsPipelineSettings()
			.SetExtent(swapchain->GetExtent())
			.SetVertexShaderModule(vertShaderModule)
			.SetFragmentShaderModule(fragShaderModule);

		m_Pipeline = swapchain->GetFramebuffers()[0]->CreateGraphicsPipeline(m_PipelineSettings);

		log::Info("Pipeline is ready : {} from scene: {}", m_Pipeline->IsReady(), GetName());

		m_VertexBuffer = device->CreateBuffer();
		m_VertexBuffer->SetUsageFlags(vk::BufferUsageFlagBits::eVertexBuffer);
		m_VertexBuffer->Resize(sizeof(VulkanVertex) * 3);
		m_VertexBuffer->SetData(vertices, sizeof(vertices));

		log::Info("MainScene started");
	}

	void MainScene::OnUpdate()
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

		if (glfwGetKey(Window::Get()->GetHandle(), GLFW_KEY_A)) {
			Application::Get()->ChangeSceneAsync("TestScene");
		}
		else if (glfwGetKey(Window::Get()->GetHandle(), GLFW_KEY_C)) {
			Application::Get()->ChangeScene("TestScene");
		}
	}

	void MainScene::OnEnd()
	{
		log::Info("MainScene ended");
		m_VertexBuffer.reset();
		m_Pipeline.reset();
	}

	void MainScene::OnPause()
	{
	}

	void MainScene::OnResume()
	{
	}

	void MainScene::OnResize(U32 width, U32 height)
	{
		(void) width; (void) height;

		const auto swapchain = Application::Get()->GetVulkanSwapchain();

		m_PipelineSettings.SetExtent(swapchain->GetExtent());
		m_Pipeline = swapchain->GetFramebuffers()[0]->CreateGraphicsPipeline(m_PipelineSettings);
	}

}