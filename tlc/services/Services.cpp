#include "services/Services.hpp"

namespace tlc
{
    std::unordered_map<Size, Scope<IService>> Services::s_Services;

    void Services::PushEvent(const String& event, const String& eventParams) 
    {
        for (auto& [_, service] : s_Services)
        {
            service->OnEvent(event, eventParams);
        }
    }

    void Services::PushSceneChangeEvent()
    {
        for (auto& [_, service] : s_Services)
        {
            service->OnSceneChange();
        }
    }
    
    void Services::Shutdown() 
    {
        for (auto& [_, service] : s_Services)
        {
            service->OnEnd();
        }
        s_Services.clear();
    }

    void Services::Setup() 
    {
        for (auto& [_, service] : s_Services)
        {
            service->OnStart();
        }
    }

}