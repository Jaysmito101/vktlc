#pragma once

#include "game/Game.hpp"
#include "game/scenes/TestScene.hpp"
#include "game/scenes/MainScene.hpp"

#include "engine/ecs/ECS.hpp"
#include "services/renderer/VulkanManager.hpp"
#include "services/renderer/PresentationRenderer.hpp"
#include "services/renderer/DebugUIManager.hpp"
#include "services/StatisticsManager.hpp"

// TODO: use a proper input manager service here rather than using glfw directly
#include "glfw/glfw3.h"

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

        // TODO: use a proper input manager service here rather than using the event manager directly
        EventManager<EventType::WindowKey, I32, I32, I32, I32>::Get()->Subscribe([this](I32 key, I32, I32 action, I32) {
            if (key == GLFW_KEY_F1 && action == GLFW_RELEASE) {
                auto debugUi = Services::Get<DebugUIManager>();
                debugUi->ToggleDebugUI();
            }
        });
    }

    void GameApplication::OnUpdate()
    {
#ifdef TLC_ENABLE_STATISTICS
        Services::Get<StatisticsManager>()->NewFrame();
#endif
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
        
        if (!IsMinimized()) {
            TLC_STATISTICS_PER_FRAME_TIME_SCOPE("GameApplication/OnUpdate/Time");
            TLC_STATISTICS_PER_FRAME("DebugUI/PrepareCPU/Time", if (Services::Get<DebugUIManager>()->IsDebugUIVisible()) RenderDebugUi();)
            TLC_STATISTICS_PER_FRAME("Render/Time", RenderEngineFrame();)
        }
#ifdef TLC_ENABLE_STATISTICS
        Services::Get<StatisticsManager>()->EndFrame();
#endif
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

    void GameApplication::RenderDebugUi() {
        auto vulkan = Services::Get<VulkanManager>();
        auto swapchain = vulkan->GetSwapchain();
        auto debugUi = Services::Get<DebugUIManager>();

        const auto& swapchainExtent = swapchain->GetExtent();
        debugUi->NewFrame(swapchainExtent.width, swapchainExtent.height, GetDeltaTime());
        
        if (debugUi->IsEditorOpen("ImGui/DemoWindow")) {
            ImGui::ShowDemoWindow(debugUi->EditorOpenPtr("ImGui/DemoWindow"));
        }

#ifdef TLC_ENABLE_STATISTICS
        if (debugUi->IsEditorOpen("Core/StatisticsManager")) {
            Services::Get<StatisticsManager>()->ShowDebugUI(debugUi->EditorOpenPtr("Core/StatisticsManager"));
        }
#endif

        debugUi->BeginEditorSection();
        ImGui::Text("Hello from the editor section!");
        debugUi->EndEditorSection();

        debugUi->EndFrame();
    }

}