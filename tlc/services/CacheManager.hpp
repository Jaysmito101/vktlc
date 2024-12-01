#pragma once

#include "core/Core.hpp"
#include "services/Services.hpp"

namespace tlc 
{
    class CacheManager : public IService {
        public:
            void Setup(const String& cachePath);
            
            void OnStart() override;
            void OnEnd() override;

            Bool CacheExists(const String& key) const;
            U64 GetCacheVersion(const String& key) const;
            void UpdateCache(const String& key, const Raw<U8> value, Size size, U64 version);
            void CreateCache(const String& key, const Raw<U8> value, Size size, U64 version);
            void ClearCache();

            List<String> GetCacheKeys() const;
            List<U8> GetCacheData(const String& key) const;
            String GetCacheDataString(const String& key) const;

            template<typename T>
            List<T> GetCacheDataTyped(const String& key) const {
                auto data = GetCacheData(key);
                return List<T>((T*)data.data(), (T*)(data.data() + data.size()));
            }

            void CacheShaders();
            void ReloadCacheMetadata();

        private:
            void SaveCache(const String& key, const Raw<U8> value, Size size, U64 version);
            void LoadCacheMetadata(const String& key);
            void LoadAllCacheMetadata();
            String FlattenKey(const String& key) const;

        private:
            String m_CachePath = "";
            UnorderedMap<String, Pair<String, U64>> m_Cache;
    };
}