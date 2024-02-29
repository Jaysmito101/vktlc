#include "game/scenes/TestScene.hpp"
#include "core/Application.hpp"


// TODO: Make input system!
#include <GLFW/glfw3.h>

namespace tlc
{
	void TestScene::OnLoad(Bool isAsync)
	{
		(void) isAsync;
		
		log::Info("Loading Scene: {}", GetName());

		std::this_thread::sleep_for(std::chrono::seconds(1));

		vertices[0].position = glm::vec4(0.0f, -0.5f, 0.0f, 1.0f);
		vertices[0].color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		vertices[1].position = glm::vec4(0.5f, 0.5f, 0.0f, 1.0f);
		vertices[1].color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
		vertices[2].position = glm::vec4(-0.5f, 0.5f, 0.0f, 1.0f);
		vertices[2].color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	}

	void TestScene::OnUnload()
	{
		
	}

	void TestScene::OnStart()
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

		log::Info("TestScene started");
	}

	void TestScene::OnUpdate()
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

		vertices[0].position = glm::vec4(0.0f, -0.5f * fabsf(sinf((F32)glfwGetTime()) + 0.05f), 0.0f, 1.0f);
		m_VertexBuffer->SetData(vertices, sizeof(F32) * 2);


		if (glfwGetKey(Window::Get()->GetHandle(), GLFW_KEY_A)) {
			Application::Get()->ChangeSceneAsync("MainScene");
		}
		else if (glfwGetKey(Window::Get()->GetHandle(), GLFW_KEY_C)) {
			Application::Get()->ChangeScene("MainScene");
		}
	}

	void TestScene::OnEnd()
	{
		log::Info("TestScene ended");
		m_VertexBuffer.reset();
		m_Pipeline.reset();
	}

	void TestScene::OnPause()
	{
	}

	void TestScene::OnResume()
	{
	}

	void TestScene::OnResize(U32 width, U32 height)
	{
		(void) width; (void) height;

		const auto swapchain = Application::Get()->GetVulkanSwapchain();

		m_PipelineSettings.SetExtent(swapchain->GetExtent());
		m_Pipeline = swapchain->GetFramebuffers()[0]->CreateGraphicsPipeline(m_PipelineSettings);
	}

}