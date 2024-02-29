#pragma once

#include "game/Game.hpp"

namespace tlc
{
    void GameApplication::OnLoad()
    {
        auto vertModule = m_VulkanDevice->CreateShaderModule(utils::ReadBinaryFie("vert.spv"));
        auto fragModule = m_VulkanDevice->CreateShaderModule(utils::ReadBinaryFie("frag.spv"));

        auto settings = VulkanGraphicsPipelineSettings()
            .SetExtent(m_VulkanSwapchain->GetExtent())
            .SetVertexShaderModule(vertModule)
            .SetFragmentShaderModule(fragModule);

        m_Pipeline = m_VulkanSwapchain->GetFramebuffers()[0]->CreateGraphicsPipeline(settings);
        log::Info("Pipeline is ready : {}", m_Pipeline->IsReady());

        m_VertexBuffer = m_VulkanDevice->CreateBuffer();
        m_VertexBuffer->SetUsageFlags(vk::BufferUsageFlagBits::eVertexBuffer);

        m_VertexBuffer->Resize(sizeof(VulkanVertex) * 3);



        vertices[0].position = glm::vec4(0.0f, -0.5f, 0.0f, 1.0f);
        vertices[0].color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
        vertices[1].position = glm::vec4(0.5f, 0.5f, 0.0f, 1.0f);
        vertices[1].color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
        vertices[2].position = glm::vec4(-0.5f, 0.5f, 0.0f, 1.0f);
        vertices[2].color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
        m_VertexBuffer->SetData(vertices, sizeof(vertices));
    }
    
    void GameApplication::OnUnload()
    {
        m_VertexBuffer.reset();
        m_Pipeline.reset();
    }
    

    void GameApplication::OnResize(U32 width, U32 height)
    {
        static Pair<U32, U32> prevWindowSize = MakePair(0u, 0u);
        if ((width != prevWindowSize.x || height != prevWindowSize.y) && !IsMinimized())
        {
            m_VulkanDevice->WaitIdle();
            m_VulkanSwapchain->Recreate();
            // TODO: Recreate the pipeline here
            prevWindowSize = MakePair(width, height);
        }
    }

    void GameApplication::OnStart()
    {
    }
    
    void GameApplication::OnUpdate()
    {
        fpsTimer += GetDeltaTime();
        fps++;
        if (fpsTimer >= 1.0f)
        {
            m_Window->SetTitle(std::format("TLC [Delta time: {}, FPS: {}]", GetDeltaTime(), fps));
            fps = 0;
            fpsTimer = 0.0f;
        }
        auto currentTime = GetCurrentFrameTime();

        //m_Window->SetTitle(std::format("TLC [Delta time: {}, FPS: {}]", m_DeltaTime, 1.0f / m_DeltaTime));

        if (!IsMinimized())
        {
            const auto currentWindowSize = m_Window->GetSize();
            m_Renderer->AcquireNextImage();

            m_Renderer->BeginFrame();

            m_Renderer->BeginDefaultRenderPass();

            // m_CommandBuffer->BindPipeline(m_Pipeline->GetPipeline());
            m_Renderer->SetPipeline(m_Pipeline.get());

            m_Renderer->SetViewport(0.0f, 0.0f, static_cast<F32>(m_VulkanSwapchain->GetExtent().width), static_cast<F32>(m_VulkanSwapchain->GetExtent().height));

            m_Renderer->SetScissor(0, 0, m_VulkanSwapchain->GetExtent().width, m_VulkanSwapchain->GetExtent().height);

            m_Renderer->GetCommandBuffer()->BindVertexBuffer(m_VertexBuffer->GetBuffer(), 0);

            m_Renderer->DrawRaw(3, 1);

            m_Renderer->EndRenderPass();
            m_Renderer->EndFrame();


            m_Renderer->PresentFrame();

            vertices[0].position = glm::vec4(sinf(static_cast<F32>(currentTime)) * 0.5f, -0.5f, 0.0f, 1.0f);
            vertices[1].position = glm::vec4(0.5f, sinf(static_cast<F32>(currentTime + 115.0f)) * 0.5f, 0.0f, 1.0f);
            vertices[2].position = glm::vec4(-0.5f, sinf(static_cast<F32>(currentTime + 230.0f)) * 0.5f, 0.0f, 1.0f);
            m_VertexBuffer->SetData(vertices, sizeof(vertices));

        }



    }

    void GameApplication::OnEnd()
    {
    }
}

