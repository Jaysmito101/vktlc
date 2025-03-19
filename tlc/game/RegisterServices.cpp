#include "game/Game.hpp"

#include "services/ShaderCompiler.hpp"
#include "services/CacheManager.hpp"
#include "services/assetmanager/AssetManager.hpp"
#include "services/assetmanager/AssetBundler.hpp"
#include "services/renderer/VulkanManager.hpp"




namespace tlc
{
    void GameApplication::RegisterServices()
    {
        Services::RegisterService<ShaderCompiler>();
        Services::RegisterService<CacheManager>(utils::GetExecutableDirectory() + "/cache");
        Services::RegisterService<AssetBundler>(utils::GetExecutableDirectory() + "/asset_bundles");
        Services::RegisterService<AssetManager>(utils::GetExecutableDirectory() + "/asset_bundles");
        Services::RegisterService<VulkanManager>();        
    }
}