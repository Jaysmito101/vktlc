#pragma once

#include "core/Core.hpp"

namespace tlc 
{
    class IService
    {
    public:
        virtual void OnStart() {};
        virtual void OnEnd() {};
        virtual void OnSceneChange() {}
        virtual void OnEvent(const String& event, const String& eventParams) {}
    };

    class Services
    {
    public:
        template<typename T, typename... Args>
        static void RegisterService(Args&&... args)
        {
            // Ensure T is a service
            static_assert(std::is_base_of<IService, T>::value, "Service must derive from IService!");

            Size hash = typeid(T).hash_code();
            TLC_ASSERT(s_Services.find(hash) == s_Services.end(), "Service already exists!");

            s_Services[hash] = CreateScope<T>();
            static_cast<Raw<T>>(s_Services[hash].get())->Setup(std::forward<Args>(args)...);
            s_Services[hash]->OnStart();
        }

        template<typename T>
        static Raw<T> GetService()
        {
            Size hash = typeid(T).hash_code();
            TLC_ASSERT(s_Services.find(hash) != s_Services.end(), "Service does not exist!");
            return static_cast<Raw<T>>(s_Services[hash].get());
        }

        static void PushEvent(const String& event, const String& eventParams);

    private:
        static void PushSceneChangeEvent();
        static void Shutdown();
        static void Setup();

        Services() = delete;
        ~Services() = delete;

    private:
        static std::unordered_map<Size, Scope<IService>> s_Services;

        friend class Application;
    };
}