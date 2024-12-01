#include "game/Game.hpp"

#include "services/assetmanager/AssetManager.hpp"
#include "services/assetmanager/AssetBundler.hpp"


namespace tlc
{
    void GameApplication::RegisterAssets()
    {
        

        // NOTE: This might seem weird(kinda is) but is very specific to the projects directory structure and how assets are managed(for now)
        auto assetsPath = String("./assets");
        if (!utils::PathExists(assetsPath))
        {
            assetsPath = "../assets";
            if (!utils::PathExists(assetsPath))
            {
                assetsPath = "../assets";
                if (!utils::PathExists(assetsPath))
                {
                    log::Warn("Game::RegisterAssets: Raw assets path does not exists! skipping manual asset registration!");
                    return;
                }
            }
        }

        auto id = UUID::Zero();
        log::Info("Zero UUID: {}", id);
        auto id2 = UUID::New();
        log::Info("New UUID: {}", id2);



        // auto bundler = Services::GetService<AssetBundler>();
        // bundler->RegisterAsset(assetsPath + "/images/test2.png", AssetTags::Image, "test_img", "test2.png");   
        // bundler->RegisterAsset(assetsPath + "/images/test2.png", AssetTags::Image | AssetTags::Font, "test_img", "tes3t2.png");   
        // bundler->RegisterAsset(assetsPath + "/images/test2.png", AssetTags::None, "test_img", "tes343t2.png");   
        // bundler->RegisterAsset(assetsPath + "/images/test2.png", AssetTags::None, "test_img", "test432.png");   
        // bundler->RegisterFromDirectory(assetsPath, "dir_assets");
        // bundler->RegisterAsset(assetsPath + "/shaders/vert.glsl", AssetTags::Shader, "shaders", "ver_t.glsl");
        // bundler->RegisterAsset(assetsPath + "/shaders/frag.glsl", AssetTags::Shader, "shaders", "fra_g.glsl");
        // bundler->LogAssets();
        // bundler->Pack();

        auto assetManager = Services::GetService<AssetManager>();
        assetManager->LogAssets();
        assetManager->LoadAllBundles();

        log::Error("Shader :\n{}", assetManager->GetAssetDataString("ver_t.glsl"));
    }
}