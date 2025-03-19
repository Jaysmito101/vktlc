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

    namespace internal {
        template <typename T>
        class ServiceStaticHolder {
        public:
            inline static Raw<T> Get() { return s_Instance.get(); }
            inline static Raw<T> Init() { 
                TLC_ASSERT(s_Instance == nullptr, "Service already exists!");
                s_Instance = CreateScope<T>();
                return s_Instance.get();
            }
        private:
            static Scope<T> s_Instance;
        };

        template <typename T>
        Scope<T> ServiceStaticHolder<T>::s_Instance = nullptr;
    }

    class Services
    {
    public:
        template<typename T, typename... Args>
        static void RegisterService(Args&&... args)
        {
            // Ensure T is a service
            static_assert(std::is_base_of<IService, T>::value, "Service must derive from IService!");
            auto service = internal::ServiceStaticHolder<T>::Init();
            service->Setup(std::forward<Args>(args)...);
            service->OnStart();
            s_Services.push_back(service);
        }

        template<typename T>
        static Raw<T> Get()
        {
            return internal::ServiceStaticHolder<T>::Get();
        }

        static void PushEvent(const String& event, const String& eventParams);

    private:
        static void PushSceneChangeEvent();
        static void Shutdown();
        static void Setup();

        Services() = delete;
        ~Services() = delete;

    private:
        static List<Raw<IService>> s_Services;

        friend class Application;
    };
}