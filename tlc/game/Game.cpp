#pragma once

#include "game/Game.hpp"
#include "game/scenes/TestScene.hpp"
#include "game/scenes/MainScene.hpp"

#include "services/ShaderCompiler.hpp"
#include "services/CacheManager.hpp"
#include "services/assetmanager/AssetManager.hpp"
#include "services/assetmanager/AssetBundler.hpp"


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

        RegisterAssets();


        // Register scenes
        // RegisterScene<TestScene>("TestScene");
        // RegisterScene<MainScene>("MainScene");
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
        auto ecs = CreateRef<ECS>();

        class System : public ISystem
        {
        public:
            virtual void OnUpdate(Raw<ECS> ecs, const Entity& entity, const UUID& component) override
            {
                (void)ecs;
                (void)entity;
                (void)component;
            }
        };
        auto system = CreateRef<System>();
        ecs->RegisterSystemWithQuery<Transform, Mesh, Material>(system, SystemTrigger::OnUpdate, 0, "TestSystem");

        auto camera = ecs->CreateEntity("Camera");
        auto light = ecs->CreateEntity("Light");

        auto world = ecs->CreateEntity("World");
        auto cube = ecs->CreateEntity("Cube", world);
        auto sphere = ecs->CreateEntity("Sphere", world);
        auto plane = ecs->CreateEntity("Plane", world);

        auto cubeRenderer = ecs->CreateEntity("CubeRenderer", cube);
        auto sphereRenderer = ecs->CreateEntity("SphereRenderer", sphere);
        auto planeRenderer = ecs->CreateEntity("PlaneRenderer", plane);

        auto cubeMesh = ecs->CreateEntity("CubeMesh", cubeRenderer);

        auto entities = ecs->CreatePath("World/Cube2/CubeRenderer2/CubeMesh2");

        auto comp = ecs->CreateComponent<Transform>(cube, "Transform"); 
        auto comp2 = ecs->CreateComponent<Mesh>(cube, "Mesh");
        auto comp3 = ecs->CreateComponent<Material>(cube, "Material");

        auto comp4 = ecs->CreateComponent<Light>(light);

        auto comp5 = ecs->CreateComponent<Transform>(plane);

        // ecs->
        log::Error("Entity Tree: ");
        ecs->PrintEntityTree();

        ecs->DestroyComponent(comp3);
        ecs->ApplyDeletions();

        log::Error("Entity Tree: ");
        ecs->PrintEntityTree();

        ecs->DestroyEntity(world);   
        ecs->ApplyDeletions();

        log::Error("Entity Tree: ");
        ecs->PrintEntityTree();

        log::Error("Systems: ");
        ecs->PrintSystems();


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

