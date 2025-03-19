#pragma once

#include "game/Game.hpp"
#include "game/scenes/TestScene.hpp"
#include "game/scenes/MainScene.hpp"


#include "engine/ecs/ECS.hpp"

#include "services/renderer/VulkanManager.hpp"


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
        auto vulkan = Services::Get<VulkanManager>();
        auto device = vulkan->GetDevice();
        m_NumInflightFrames = std::clamp(vulkan->GetSwapchain()->GetImageCount(), (Size)1, (Size)2);
        for (Size i = 0; i < m_NumInflightFrames; i++)
        {
            m_ImageAvailableSemaphores.push_back(device->CreateVkSemaphore());
            m_RenderFinishedSemaphores.push_back(device->CreateVkSemaphore());
            m_InFlightFences.push_back(device->CreateVkFence());
        }
        m_CurrentFrameIndex = 0;

        // ChangeScene("TestScene");

    }
    
    void GameApplication::OnUpdate()
    {
        // the scene has already been updated
        // and so have been the systems
        // so we can assume the scene
        // has been renderer and can
        // and we can fetch what to show on
        // screen fromt he active camera
        // also any sort of update to be done in the debug ui layer 
        // (imgui side building of command buffers) should be done here


        // update the vulkan manager that acquires
        // a new image from the swapchain
        // and draws the scene and also 
        // draws the debug ui layer (using the vulkan imgui renderer)
        RenderEngineFrame();

    }

    void GameApplication::OnEnd()
    {
        auto vulkan = Services::Get<VulkanManager>();
        auto device = vulkan->GetDevice();
        for (Size i = 0; i < m_NumInflightFrames; i++)
        {
            device->DestroyVkSemaphore(m_ImageAvailableSemaphores[i]);
            device->DestroyVkSemaphore(m_RenderFinishedSemaphores[i]);
            device->DestroyVkFence(m_InFlightFences[i]);
        }
        m_ImageAvailableSemaphores.clear();
        m_RenderFinishedSemaphores.clear();
        m_InFlightFences.clear();
    }

    void GameApplication::RenderEngineFrame() {
        auto vulkan = Services::Get<VulkanManager>();
        auto device = vulkan->GetDevice();
        auto swapchain = vulkan->GetSwapchain();
        
        VkCall(device->GetDevice().waitForFences({m_InFlightFences[m_CurrentFrameIndex]}, VK_TRUE, UINT64_MAX));
        device->GetDevice().resetFences({m_InFlightFences[m_CurrentFrameIndex]});

        auto imageIndex = swapchain->AcquireNextImage(m_ImageAvailableSemaphores[m_CurrentFrameIndex]);

        // auto frame = GetCurrentScene()->GetActiveCamera()->GetFrame();
        // auto frameRenderedSemaphore = frame.GetFrameRenderedSemaphore();

        // do the actual rendering here
        // also the debug ui rendering
        // wait on both the frame rendered semaphore(to start rendering) and the image available semaphore(for wrting color output)
        // and signal the render finished semaphore
        // and also set signal fense to the inflight fence


        swapchain->PresentImage(imageIndex, m_RenderFinishedSemaphores[m_CurrentFrameIndex]);
        m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % m_NumInflightFrames;        
    }
}

