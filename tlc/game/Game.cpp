#pragma once

#include "game/Game.hpp"
#include "game/scenes/TestScene.hpp"
#include "game/scenes/MainScene.hpp"

#include "services/ShaderCompiler.hpp"




namespace tlc
{
    void GameApplication::OnLoad()
    {
        // Register services
        Services::RegisterService<ShaderCompiler>();

        // Register scenes
        RegisterScene<TestScene>("TestScene");
        RegisterScene<MainScene>("MainScene");
    }
    
    void GameApplication::OnUnload()
    {
           
    }
    

    void GameApplication::OnResize(U32 width, U32 height)
    {
        (void)width; (void)height;
    }

    void GameApplication::OnStart()
    {
        ChangeScene("TestScene");
    }
    
    void GameApplication::OnUpdate()
    {
        fpsTimer += GetDeltaTime();
        fps++;
        if (fpsTimer >= 1.0f)
        {
            m_Window->SetTitle(std::format("TLC [Delta time: {}, FPS: {}, Scene: {}]", GetDeltaTime(), fps, GetCurrentScene()->GetName()));
            fps = 0;
            fpsTimer = 0.0f;
        }


    }

    void GameApplication::OnEnd()
    {
    }
}

