#pragma once

#include "game/Game.hpp"
#include "game/scenes/TestScene.hpp"
#include "game/scenes/MainScene.hpp"

#include "engine/ecs/ECS.hpp"
#include "services/renderer/VulkanManager.hpp"
#include "services/renderer/PresentationRenderer.hpp"
#include "services/renderer/DebugUIManager.hpp"

namespace tlc
{
    void GameApplication::OnLoad()
    {
        RegisterServices();
        RegisterAssets();

        // Register scenes
        // RegisterScene<TestScene>("TestScene");
        // RegisterScene<MainScene>("MainScene");
    }

    void GameApplication::OnUnload()
    {
    }

    void GameApplication::OnStart()
    {
        // ChangeScene("TestScene");
    }

    void GameApplication::OnUpdate()
    {
        if (GetCurrentFrameTime() - m_LastFrameTime > 1.0f)
        {
            m_LastFrameTime = GetCurrentFrameTime();
            Window::Get()->SetTitle("TLC - " + std::to_string(GetCurrentFramerate()) + " FPS");
        }

        // the scene has already been updated
        // and so have been the systems
        // so we can assume the scene
        // has been renderer and can
        // and we can fetch what to show on
        // screen fromt he active camera
        // also any sort of update to be done in the debug ui layer
        // (imgui side building of command buffers) should be done here

        // TODO: Move this to a proper system!
        auto vulkan = Services::Get<VulkanManager>();
        auto swapchain = vulkan->GetSwapchain();
        auto debugUi = Services::Get<DebugUIManager>();
        const auto& swapchainExtent = swapchain->GetExtent();
        debugUi->NewFrame(swapchainExtent.width, swapchainExtent.height, GetDeltaTime());
        ImGui::ShowDemoWindow();


        debugUi->EndFrame();

        // update the vulkan manager that acquires
        // a new image from the swapchain
        // and draws the scene and also
        // draws the debug ui layer (using the vulkan imgui renderer)
        if (!IsMinimized()) RenderEngineFrame();
    }

    void GameApplication::OnEnd()
    {
    }

    void GameApplication::RenderEngineFrame()
    {
        auto presentationRenderer = Services::Get<PresentationRenderer>();
        if(!presentationRenderer->RenderCurrentFrame(GetDeltaTime())) {
            log::Warn("Engine frame skipped...");
        }
    }

}