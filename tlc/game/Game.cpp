#pragma once

#include "game/Game.hpp"
#include "game/scenes/TestScene.hpp"
#include "game/scenes/MainScene.hpp"

#include "services/ShaderCompiler.hpp"
#include "services/CacheManager.hpp"
#include "services/assetmanager/AssetManager.hpp"
#include "services/assetmanager/AssetBundler.hpp"
#include "services/renderer/VulkanManager.hpp"


#include "engine/ecs/ECS.hpp"


struct Transform {};
struct Mesh {};
struct Material {};
struct Light {};


namespace tlc
{
    void GameApplication::OnLoad()
    {
        // Register services
        Services::RegisterService<ShaderCompiler>();
        Services::RegisterService<CacheManager>(utils::GetExecutableDirectory() + "/cache");
        Services::RegisterService<AssetBundler>(utils::GetExecutableDirectory() + "/asset_bundles");
        Services::RegisterService<AssetManager>(utils::GetExecutableDirectory() + "/asset_bundles");
        Services::RegisterService<VulkanManager>();

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
        fpsTimer += GetDeltaTime();
        fps++;
        if (fpsTimer >= 1.0f)
        {
            
            //Window::Get()->SetTitle(std::format("TLC [Delta time: {}, FPS: {}, Scene: {}]", GetDeltaTime(), fps, GetCurrentScene()->GetName()));
            fps = 0;
            fpsTimer = 0.0f;
        }


    }

    void GameApplication::OnEnd()
    {
    }
}

