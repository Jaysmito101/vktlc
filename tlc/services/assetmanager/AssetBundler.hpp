#pragma once

#include "core/Core.hpp"
#include "services/Services.hpp"
#include "services/assetmanager/Asset.hpp"

namespace tlc 
{
    class AssetBundler : public IService {
        public:

            void Setup(const String& bundlesPath);

            Bool RegisterAsset(
                const String& path,
                AssetTags tags,
                const String& bundleName,
                const String& address 
            );
            Bool RegisterFromDirectory(const String& path, const String& bundleName, const String& addressPrefix = "");
            Bool AssetExists(const String& address); 
            void Pack();

            
            void OnStart() override;
            void OnEnd() override;

            void LogAssets();

        private:
            AssetTags DetectAssetTags(const String& path);
            void PackBundle(const String& bundleName);
            void LoadAssets(const String& bundleName);
            void UnloadAssets(const String& bundleName);
            void WriteAssetMetadata(std::ofstream& bundleFile, const Asset& asset);

        private:
            std::mutex m_Mutex;
            UnorderedMap<String, List<Asset>> m_Assets;
            String m_BundlesPath = "";
    };
}