#include "services/CacheManager.hpp"
#include "services/ShaderCompiler.hpp"
#include "services/assetmanager/AssetManager.hpp"

namespace tlc {
    void CacheManager::Setup(const String& cachePath) {
        m_CachePath = cachePath;
    }

    void CacheManager::OnStart() {
        utils::EnsureDirectory(m_CachePath);
        ReloadCacheMetadata();
    }

    void CacheManager::OnEnd() {
    }

    void CacheManager::ReloadCacheMetadata() {
        m_Cache.clear();
        LoadAllCacheMetadata();
    }

    Bool CacheManager::CacheExists(const String& key) const {
        return m_Cache.find(key) != m_Cache.end();
    }

    U64 CacheManager::GetCacheVersion(const String& key) const {
        auto cache = m_Cache.find(key);
        if (cache == m_Cache.end()) {
            return 0;
        }
        return cache->second.second;
    }

    void CacheManager::UpdateCache(const String& key, const Raw<U8> value, Size size, U64 version) {
        auto cache = m_Cache.find(key);
        if (cache == m_Cache.end()) {
            log::Warn("Cache with key: {} does not exist!", key);
            return;
        }

        if (cache->second.second != version) {
            SaveCache(key, value, size, version);
        }
        else {
            log::Warn("Cache with key: {} is already up to date!", key);
        }
    }

    void CacheManager::CreateCache(const String& key, const Raw<U8> value, Size size, U64 version) {
        SaveCache(key, value, size, version);
    }

    void CacheManager::ClearCache() {
        m_Cache.clear();
        for (auto files : std::filesystem::directory_iterator(m_CachePath)) {
            auto path = files.path();
            if (path.extension() == ".cache") {
                std::remove(path.string().c_str());
            }
        }
    }

    List<String> CacheManager::GetCacheKeys() const {
        List<String> result;
        for (const auto& [key, _] : m_Cache) {
            result.emplace_back(key);
        }
        return result;
    }

    List<U8> CacheManager::GetCacheData(const String& key) const {
        auto cache = m_Cache.find(key);
        if (cache == m_Cache.end()) {
            return {};
        }

        auto cachePath = cache->second.first;
        std::ifstream cacheFile(cachePath, std::ios::binary);
        if (!cacheFile.is_open()) {
            log::Error("Failed to open cache file: {}", cachePath);
            return {};
        }

        // read the hash
        U32 hash = 0;
        cacheFile.read(reinterpret_cast<char*>(&hash), sizeof(U32));
        // read the size
        Size size = 0;
        cacheFile.read(reinterpret_cast<char*>(&size), sizeof(Size));
        // read the version
        U64 version = 0;
        cacheFile.read(reinterpret_cast<char*>(&version), sizeof(U64));

        // read the key
        static char keyBuffer[1024];
        cacheFile.read(keyBuffer, 1024);

        auto data = List<U8>(size);
        cacheFile.read(reinterpret_cast<char*>(data.data()), size);
        cacheFile.close();

        return data;
    }

    String CacheManager::GetCacheDataString(const String& key) const {
        auto data = GetCacheData(key);
        return std::string(data.begin(), data.end());
    }

    void CacheManager::SaveCache(const String& key, const Raw<U8> value, Size size, U64 version) {
        auto hash = utils::HashBuffer(value, size);
        auto cachePath = m_CachePath + "/" + FlattenKey(key) + ".cache";
        std::ofstream cacheFile(cachePath, std::ios::binary);
        if (!cacheFile.is_open()) {
            log::Error("Failed to create cache file: {}", cachePath);
            return;
        }

        cacheFile.write(reinterpret_cast<const char*>(&hash), sizeof(U32));
        cacheFile.write(reinterpret_cast<const char*>(&size), sizeof(Size));
        cacheFile.write(reinterpret_cast<const char*>(&version), sizeof(U64));
        static char keyBuffer[1024];
        std::snprintf(keyBuffer, 1024, "%s", key.c_str());
        cacheFile.write(keyBuffer, 1024);
        cacheFile.write(reinterpret_cast<const char*>(value), size);
        cacheFile.close();

        m_Cache[key] = { cachePath, version };
    }
    
    void CacheManager::LoadCacheMetadata(const String& cachePath)
    {
        if (!utils::PathExists(cachePath)) {
            log::Warn("Cache: {} not found!", cachePath);
            return;
        }

        // open the cache file
        std::ifstream cacheFile(cachePath, std::ios::binary);
        if (!cacheFile.is_open()) {
            log::Error("Failed to open cache file: {}", cachePath);
            return;
        }

        // read the hash
        U32 hash = 0;
        cacheFile.read(reinterpret_cast<char*>(&hash), sizeof(U32));
        // read the size
        Size size = 0;
        cacheFile.read(reinterpret_cast<char*>(&size), sizeof(Size));
        // read the version
        U64 version = 0;
        cacheFile.read(reinterpret_cast<char*>(&version), sizeof(U64));
        // read the key
        static char keyBuffer[1024];
        cacheFile.read(keyBuffer, 1024);
        auto key = String(keyBuffer);
        cacheFile.close();

        m_Cache[key] = { String(cachePath), version };
    }

    void CacheManager::LoadAllCacheMetadata()
    {
        for (auto files : std::filesystem::directory_iterator(m_CachePath)) {
            auto path = files.path();
            if (path.extension() == ".cache") {
                LoadCacheMetadata(path.string());
            }
        }
    }

    void CacheManager::CacheShaders() {
        auto shaderCompiler = Services::Get<ShaderCompiler>();
        auto assetManager = Services::Get<AssetManager>();

        if (!shaderCompiler || !assetManager) {
            log::Warn("CacheManager::CacheShaders: ShaderCompiler or AssetManager service not found!");
            return;
        }

        auto lists = {
            std::make_pair(assetManager->GetAssetsWithTags(AssetTags::VertexShader), ShaderCompiler::ShaderType::Vertex),
            std::make_pair(assetManager->GetAssetsWithTags(AssetTags::FragmentShader), ShaderCompiler::ShaderType::Fragment),
            std::make_pair(assetManager->GetAssetsWithTags(AssetTags::ComputeShader), ShaderCompiler::ShaderType::Compute)
        };

        for (const auto& [list, type] : lists) {
            for (const auto& address : list) {
                Bool requiresUpdate = false;
                if (CacheExists(address)) {
                    auto version = GetCacheVersion(address);
                    auto assetHash = assetManager->GetAssetDataHash(address);
                    requiresUpdate = version != assetHash;
                }
                else {
                    requiresUpdate = true;
                }

                if (!requiresUpdate) {
                    continue;
                }

                log::Info("Compiling and caching shader: {}", address);

                auto code = assetManager->GetAssetDataString(address);
                auto spv = shaderCompiler->ToSpv(code, type, address);
                if (spv.empty()) {
                    log::Error("CacheManager::CacheShaders: failed to cache shader: {}", address);
                    continue;
                }

                CreateCache(address, reinterpret_cast<Raw<U8>>(spv.data()), spv.size() * sizeof(U32), assetManager->GetAssetDataHash(address));
            }
        }
    }

    String CacheManager::FlattenKey(const String& key) const {
        // replace all invalid characters with '_'
        String result = key;
        std::replace_if(result.begin(), result.end(), [](char c) { return !std::isalnum(c); }, '_');
        return result;
    }
}