#include "services/assetmanager/AssetManager.hpp"

namespace tlc {

    void AssetManager::Setup(const String& bundlesPath) {
        m_BundlesPath = bundlesPath;
    }

    void AssetManager::OnStart() {
        utils::EnsureDirectory(m_BundlesPath);
        ReloadAssetMetadata();
        LoadAllBundles();
    }

    void AssetManager::OnEnd() {
        UnloadAllBundles();
    }

    void AssetManager::ReadAssetMetadata(std::ifstream& bundleFile, Asset& asset) {
        // read the uuid
        bundleFile.read(reinterpret_cast<char*>(&asset.UUID), sizeof(UUID));
        // read the size
        bundleFile.read(reinterpret_cast<char*>(&asset.Size), sizeof(Size));
        // read the offset
        bundleFile.read(reinterpret_cast<char*>(&asset.Offset), sizeof(Size));
        // read the tags
        bundleFile.read(reinterpret_cast<char*>(&asset.Tags), sizeof(AssetTags));
        // read the hash
        bundleFile.read(reinterpret_cast<char*>(&asset.Hash), sizeof(U32));
        // read the address [max 1024 bytes]
        static char address[1024];
        bundleFile.read(address, 1024);
        asset.Address = address;
    }


    void AssetManager::LoadBundleMetadata(const String& bundleName) {
        auto bundlePath = m_BundlesPath + "/" + bundleName + ".bundle";
        if (!utils::PathExists(bundlePath)) {
            log::Warn("Bundle: {} not found!", bundlePath);
            return;
        }

        // open the bundle file
        std::ifstream bundleFile(bundlePath, std::ios::binary);
        if (!bundleFile.is_open()) {
            log::Error("Failed to open bundle file: {}", bundlePath);
            return;
        }

        // read the number of assets
        U32 numAssets = 0;
        bundleFile.read(reinterpret_cast<char*>(&numAssets), sizeof(U32));
        
        auto assets = List<Asset>();
        assets.reserve(numAssets);

        for (U32 i = 0; i < numAssets; i++) {
            Asset asset;
            ReadAssetMetadata(bundleFile, asset);
            assets.emplace_back(asset);
        }

        // store the assets
        m_Assets[bundleName] = { nullptr, assets };
    }

    void AssetManager::ReloadAssetMetadata() 
    {
        UnloadAllBundles();

        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Assets.clear();

        // load all the bundle metadata
        for (const auto& entry : std::filesystem::directory_iterator(m_BundlesPath)) {
            auto path = entry.path();
            if (path.extension() != ".bundle") {
                continue;
            }

            log::Info("Loading bundle: {}", path.string());
            auto bundleName = path.stem().string();
            LoadBundleMetadata(bundleName);
        }
    }

    void AssetManager::UnloadBundle(const String& bundleName)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        auto bundle = m_Assets.find(bundleName);
        if(bundle == m_Assets.end()) {
            log::Warn("Bundle: {} not found!", bundleName);
            return;
        }

        // unload the assets
        if (bundle->second.first != nullptr) {
            // delink the assets
            for (auto& asset : bundle->second.second) {
                asset.Data = nullptr;
            }

            // free the memory
            delete[] bundle->second.first;
            bundle->second.first = nullptr;
        }
    }

    void AssetManager::UnloadAllBundles()
    {
        for (const auto& [bundleName, _] : m_Assets) {
            UnloadBundle(bundleName);
        }
    }

    void AssetManager::LogAssets()
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        for (const auto& [bundleName, bundle] : m_Assets) {
            log::Trace("Bundle: {}", bundleName);
            for (const auto& asset : bundle.second) {
                log::Trace("Asset: {} | Address: {} | Tags: {} | Offset: {}",
                    asset.Path, asset.Address, asset.Tags, asset.Offset
                );
            }
        }
    }

    void AssetManager::LoadBundle(const String& bundleName)
    {
        if (bundleName.empty()) {
            log::Warn("Bundle name is empty!");
            return;
        }

        auto bundle = m_Assets.find(bundleName);
        if(bundle == m_Assets.end()) {
            log::Warn("Bundle: {} not found!", bundleName);
            return;
        }

        if (bundle->second.first != nullptr) {
            log::Warn("Bundle: {} already loaded!", bundleName);
            return;
        }

        std::lock_guard<std::mutex> lock(m_Mutex);
        auto file = m_BundlesPath + "/" + bundleName + ".bundle";
        auto bundleFile = std::ifstream(file, std::ios::binary);
        if (!bundleFile.is_open()) {
            log::Error("Failed to open bundle file: {}", file);
            return;
        }

        // read the whole bundle into memory
        bundleFile.seekg(0, std::ios::end);
        auto size = bundleFile.tellg();
        bundleFile.seekg(0, std::ios::beg);

        bundle->second.first = new U8[size];
        bundleFile.read(reinterpret_cast<char*>(bundle->second.first), size);
        bundleFile.close();


        // link the assets
        auto& assets = bundle->second.second;
        for (auto& asset : assets) {
            asset.Data = bundle->second.first + asset.Offset;
        }

        log::Info("Bundle: {} loaded!", bundleName);
    }
    
    void AssetManager::LoadAllBundles()
    {
        for (const auto& [bundleName, _] : m_Assets) {
            LoadBundle(bundleName);
        }
    }

    List<String> AssetManager::GetBundleNames() const {
        List<String> result;
        for (const auto& [bundleName, _] : m_Assets) {
            result.emplace_back(bundleName);
        }
        return result;
    }

    List<String> AssetManager::GetAllAssets() const {
        List<String> result;
        for (const auto& [_, bundle] : m_Assets) {
            for (const auto& asset : bundle.second) {
                result.emplace_back(asset.Address);
            }
        }
        return result;
    }

    Bool AssetManager::AssetExists(const String& address) const {
        String bundleName = "";
        auto asset = GetAsset(address, bundleName);
        return asset.has_value();
    }

    Bool AssetManager::AssetLoaded(const String& address) const
    {
        String bundleName = "";
        auto asset = GetAsset(address, bundleName);
        if (!asset.has_value()) {
            return false;
        }
        
        auto bundle = m_Assets.find(bundleName);
        if(bundle == m_Assets.end()) {
            log::Warn("Bundle: {} not found!", bundleName);
            return false;
        }

        return bundle->second.first != nullptr;        
    }

    String AssetManager::GetAssetBundle(const String& address) const
    {
        String bundleName = "";
        GetAsset(address, bundleName);
        return bundleName;
    }

    List<String> AssetManager::GetAssetsInBundle(const String& bundleName) const
    {
        List<String> result;
        auto bundle = m_Assets.find(bundleName);
        if(bundle == m_Assets.end()) {
            log::Warn("Bundle: {} not found!", bundleName);
            return result;
        }

        for (const auto& asset : bundle->second.second) {
            result.emplace_back(asset.Address);
        }

        return result;
    }

    List<String> AssetManager::GetAssetsWithTags(AssetTags tags) const {
        List<String> result;
        for (const auto& [_, bundle] : m_Assets) {
            for (const auto& asset : bundle.second) {
                if ((asset.Tags & tags) == tags) {
                    result.emplace_back(asset.Address);
                }
            }
        }
        return result;
    }

    List<String> AssetManager::GetAssetsWithTagsInBundle(AssetTags tags, const String& bundleName) const
    {
        List<String> result;
        auto bundle = m_Assets.find(bundleName);
        if(bundle == m_Assets.end()) {
            log::Warn("Bundle: {} not found!", bundleName);
            return result;
        }

        for (const auto& asset : bundle->second.second) {
            if ((asset.Tags & tags) == tags) {
                result.emplace_back(asset.Address);
            }
        }

        return result;
    }

    const std::optional<Asset> AssetManager::GetAsset(const String& address, String& bundleName) const {
        bundleName = "";
        for (const auto& [assetBundleName, bundle] : m_Assets) {
            for (const auto& asset : bundle.second) {
                if (asset.Address == address) {
                    bundleName = assetBundleName;
                    return std::optional<Asset>(asset);
                }
            }
        }
        return std::nullopt;
    }

    const Raw<U8> AssetManager::GetAssetDataRaw(const String& address, Size& size) const {
        String bundleName = "";
        auto asset = GetAsset(address, bundleName);
        if (!asset.has_value()) {
            log::Warn("Asset: {} not found!", address);
            return nullptr;
        }
        size = asset->Size;
        return asset->Data;
    }

    String AssetManager::GetAssetDataString(const String& address) const  {
        String bundleName = "";
        auto asset = GetAsset(address, bundleName);
        if (!asset.has_value()) {
            log::Warn("Asset: {} not found!", address);
            return "";
        }

        return String(reinterpret_cast<const char*>(asset->Data), asset->Size);
    }

    U32 AssetManager::GetAssetDataHash(const String& address) const {
        String bundleName = "";
        auto asset = GetAsset(address, bundleName);
        if (!asset.has_value()) {
            log::Warn("Asset: {} not found!", address);
            return 0;
        }

        return asset->Hash;
    }
}
