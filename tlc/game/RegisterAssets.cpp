#include "game/Game.hpp"

#include "services/assetmanager/AssetManager.hpp"
#include "services/assetmanager/AssetBundler.hpp"
#include "services/CacheManager.hpp"


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
                assetsPath = "../../assets";
                if (!utils::PathExists(assetsPath))
                {
                    log::Warn("Game::RegisterAssets: Raw assets path does not exists! skipping manual asset registration!");
                    return;
                }
            }
        }



        auto bundler = Services::Get<AssetBundler>();
        bundler->RegisterFromDirectory(assetsPath + "/standard", "standard");
        bundler->RegisterFromDirectory(assetsPath + "/debug", "debug");
        bundler->LogAssets();
        bundler->Pack();

        auto assetManager = Services::Get<AssetManager>();
        assetManager->ReloadAssetMetadata();
        assetManager->LogAssets();
        assetManager->LoadAllBundles();

        auto cacheManager = Services::Get<CacheManager>();
        cacheManager->CacheShaders();

    }
}