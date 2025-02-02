#pragma once

#include "services/Services.hpp"
#include "imgui.h"

namespace tlc {

    class DebugUIManager : public IService {
    public:
        void Setup();
        virtual void OnStart() override;
        virtual void OnEnd() override;
        virtual void OnSceneChange() override;
        virtual void OnEvent(const String& event, const String& eventParams) override;

    private:
        void SetupForVulkan();
    };

}