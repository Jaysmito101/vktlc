#include "services/assetmanager/AssetBundler.hpp"

namespace tlc {

    void AssetBundler::Setup(const String& bundlesPath) {
        m_BundlesPath = bundlesPath;
    }

    void AssetBundler::OnStart() {
        utils::EnsureDirectory(m_BundlesPath);
    }

    void AssetBundler::OnEnd() {

    }

    Bool AssetBundler::AssetExists(const String& address)
    {       
        for (const auto& [_, bundle] : m_Assets) {
            for (const auto& asset : bundle) {
                if (asset.Address == address) {
                    return true;
                }
            }
        }
        return false;
    }

    Bool AssetBundler::RegisterAsset(
        const String& path,
        AssetTags tags,
        const String& bundleName,
        const String& address
    ) {
        std::lock_guard<std::mutex> lock(m_Mutex);

        if(AssetExists(address)) {
            log::Warn("An asset with address : {} already exists!", address);
            return false;
        }

        if (!utils::PathExists(path)) {
            log::Warn("Asset not found at path: {}!", path);
            return false;
        }

        auto res = m_Assets.find(bundleName);
        if(res == m_Assets.end()) {
            m_Assets[bundleName] = List<Asset>();
            res = m_Assets.find(bundleName);
        }

        res->second.emplace_back(Asset{
            .Path = path,
            .Address = address,
            .UUID = UUID::New(),
            .Data = nullptr,
            .Offset = 0,
            .Size = 0,
            .Tags = tags,
            .Hash = 0,
        });

        return true;
    }

    void AssetBundler::LogAssets()
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        for (const auto& [bundleName, bundle] : m_Assets) {
            log::Trace("Bundle: {}", bundleName);
            for (const auto& asset : bundle) {
                log::Trace("Asset: {} | Address: {} | Tags: {}",
                    asset.Path, asset.Address, asset.Tags
                );
            }
        }
    }

    Bool AssetBundler::RegisterFromDirectory(const String& path, const String& bundleName, const String& addressPrefix) 
    {
        if (!utils::PathExists(path)) {
            log::Warn("Directory not found at path: {}!", path);
            return false;
        }

        // recursively add all files in the directory
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (entry.is_regular_file()) {
                auto filePath = entry.path().string();
                auto address = addressPrefix + entry.path().filename().string();
                auto tags = DetectAssetTags(filePath);
                if(!RegisterAsset(filePath, tags, bundleName, address)) {
                    log::Warn("Failed to register asset: {} | Address: {}", filePath, address);
                }
            }
            else if (entry.is_directory()) {
                RegisterFromDirectory(entry.path().string(), bundleName, addressPrefix + entry.path().filename().string() + "/");
            }
        }        

        return true;
    }

    AssetTags AssetBundler::DetectAssetTags(const String& path) {
        auto tags = AssetTags::None;

        if (path.ends_with(".png") || path.ends_with(".jpg") || path.ends_with(".jpeg")) {
            tags = tags | AssetTags::Image;
        }
        else if (path.ends_with(".wav") || path.ends_with(".mp3") || path.ends_with(".ogg")) {
            tags = tags | AssetTags::Audio;
        }
        else if (path.ends_with(".ttf") || path.ends_with(".otf")) {
            tags = tags | AssetTags::Font;
        }
        else if (path.ends_with(".glsl")) {
            tags = tags | AssetTags::Shader;
            if (path.find("vert.glsl") != String::npos) {
                tags = tags | AssetTags::VertexShader;
            }
            else if (path.find("frag.glsl") != String::npos) {
                tags = tags | AssetTags::FragmentShader;
            }
            else if (path.find("comp.glsl") != String::npos) {
                tags = tags | AssetTags::ComputeShader;
            }
        }
        return tags;
    }

    void AssetBundler::LoadAssets(const String& bundleName) 
    {
        auto bundle = m_Assets.find(bundleName);
        if(bundle == m_Assets.end()) {
            log::Warn("Bundle: {} not found!", bundleName);
            return;
        }

        auto& assets = bundle->second;

        for (auto& asset : assets) {
            asset.Data = nullptr;
            asset.Size = 0;

            std::ifstream assetFile(asset.Path, std::ios::binary);
            if (!assetFile.is_open()) {
                log::Warn("Failed to open asset file: {}", asset.Path);
                continue;                
            }

            // get the size of the file
            assetFile.seekg(0, std::ios::end);
            asset.Size = assetFile.tellg();
            assetFile.seekg(0, std::ios::beg);

            // allocate memory for the asset
            asset.Data = new U8[asset.Size];
            assetFile.read(reinterpret_cast<char*>(asset.Data), asset.Size);
            assetFile.close();

            // calculate the hash of the asset
            asset.Hash = utils::HashBuffer(asset.Data, asset.Size);
        }
    }

    void AssetBundler::UnloadAssets(const String& bundleName)
    {
        auto bundle = m_Assets.find(bundleName);
        if(bundle == m_Assets.end()) {
            log::Warn("Bundle: {} not found!", bundleName);
            return;
        }

        auto& assets = bundle->second;

        for (auto& asset : assets) {
            if (asset.Data != nullptr) {
                delete[] asset.Data;
                asset.Data = nullptr;
                asset.Size = 0;
            }
        }
    }

    void AssetBundler::WriteAssetMetadata(std::ofstream& bundleFile, const Asset& asset)
    {
        // write the uuid
        bundleFile.write(reinterpret_cast<const char*>(&asset.UUID), sizeof(UUID));
        // write the size
        bundleFile.write(reinterpret_cast<const char*>(&asset.Size), sizeof(Size));
        // write the offset
        bundleFile.write(reinterpret_cast<const char*>(&asset.Offset), sizeof(Size));
        // write the tags
        bundleFile.write(reinterpret_cast<const char*>(&asset.Tags), sizeof(AssetTags));
        // write the hash
        bundleFile.write(reinterpret_cast<const char*>(&asset.Hash), sizeof(U32));
        // write the address [max 1024 bytes]
        static char address[1024];
        std::snprintf(address, 1024, "%s", asset.Address.c_str());
        bundleFile.write(address, 1024);
    }

    void AssetBundler::PackBundle(const String& bundleName)
    {
        auto bundlePath = m_BundlesPath + "/" + bundleName + ".bundle"; 
        auto bundle = m_Assets.find(bundleName);
        if(bundle == m_Assets.end()) {
            log::Warn("Bundle: {} not found!", bundleName);
            return;
        }

        auto& assets = bundle->second;

        // create the bundle file
        std::ofstream bundleFile(bundlePath, std::ios::binary);
        if (!bundleFile.is_open()) {
            log::Error("Failed to create bundle file: {}", bundlePath);
            return;
        }

        // write the number of assets in the bundle
        U32 numAssets = static_cast<U32>(assets.size());
        bundleFile.write(reinterpret_cast<const char*>(&numAssets), sizeof(U32));

        LoadAssets(bundleName);

        auto offset = sizeof(U32) + numAssets * (sizeof(UUID) + sizeof(Size) + sizeof(Size) + sizeof(AssetTags) + sizeof(U32) + 1024);

        // write the asset metadata
        for (auto& asset : assets) {
            asset.Offset = offset;
            offset += asset.Size;

            WriteAssetMetadata(bundleFile, asset);
        }

        // write the asset data
        for (const auto& asset : assets) {
            bundleFile.write(reinterpret_cast<const char*>(asset.Data), asset.Size);
        }

        UnloadAssets(bundleName);

        bundleFile.close();
    }

    void AssetBundler::Pack() 
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        for (const auto& [bundleName, bundle] : m_Assets) {
            PackBundle(bundleName);
        }
    }
}