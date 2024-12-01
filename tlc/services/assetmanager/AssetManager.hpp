#pragma once

#include "core/Core.hpp"
#include "services/Services.hpp"
#include "services/assetmanager/Asset.hpp"

namespace tlc 
{
    class AssetManager : public IService {
        public:
            void Setup(const String& bundlesPath);

            void OnStart() override;
            void OnEnd() override;
            void ReloadAssetMetadata();
            void UnloadBundle(const String& bundleName);
            void UnloadAllBundles();
            void LoadBundle(const String& bundleName);
            void LoadAllBundles();
            void LogAssets();

            // Asset queries
            List<String> GetBundleNames() const;
            Bool AssetExists(const String& address) const;
            Bool AssetLoaded(const String& address) const;
            String GetAssetBundle(const String& address) const;
            List<String> GetAllAssets() const;
            List<String> GetAssetsInBundle(const String& bundleName) const;
            List<String> GetAssetsWithTags(AssetTags tags) const;
            List<String> GetAssetsWithTagsInBundle(AssetTags tags, const String& bundleName) const;


            // Asset Data queries
            const Raw<U8> GetAssetDataRaw(const String& address, Size& size) const;
            String GetAssetDataString(const String& address) const;
            U32 GetAssetDataHash(const String& address) const;


        private:
            void ReadAssetMetadata(std::ifstream& bundleFile, Asset& asset);
            void LoadBundleMetadata(const String& bundleName);

            const std::optional<Asset> GetAsset(const String& address, String& bundle) const;

        private:
            std::mutex m_Mutex;
            UnorderedMap<String, Pair<Raw<U8>, List<Asset>>> m_Assets;
            String m_BundlesPath = "";
    };
}