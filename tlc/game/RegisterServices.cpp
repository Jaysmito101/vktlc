#include "game/Game.hpp"

#include "services/ShaderCompiler.hpp"
#include "services/CacheManager.hpp"
#include "services/assetmanager/AssetManager.hpp"
#include "services/assetmanager/AssetBundler.hpp"
#include "services/renderer/VulkanManager.hpp"
#include "services/renderer/PresentationRenderer.hpp"




namespace tlc
{
    void GameApplication::RegisterServices()
    {
        // NOTE: The order of registration matters
        // as some services depend on others to be registered first
        Services::RegisterService<ShaderCompiler>();
        Services::RegisterService<CacheManager>(utils::GetExecutableDirectory() + "/cache");
        Services::RegisterService<AssetBundler>(utils::GetExecutableDirectory() + "/asset_bundles");
        Services::RegisterService<AssetManager>(utils::GetExecutableDirectory() + "/asset_bundles");
        Services::RegisterService<VulkanManager>();  
        Services::RegisterService<PresentationRenderer>();      
    }
}