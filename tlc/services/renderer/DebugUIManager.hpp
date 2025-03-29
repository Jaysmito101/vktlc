#pragma once

#include "imgui.h"

#include "services/Services.hpp"
#include "vulkanapi/VulkanContext.hpp"
#include "vulkanapi/VulkanDevice.hpp"
#include "vulkanapi/VulkanImage.hpp"

namespace tlc {

    class DebugUIManager : public IService {
    public:
        void Setup();
        virtual void OnStart() override;
        virtual void OnEnd() override;
        virtual void OnSceneChange() override;
        virtual void OnEvent(const String& event, const String& eventParams) override;

        inline const Raw<ImFont> GetFont(const String& name) const {
            auto font = m_Fonts.find(name);
            if (font != m_Fonts.end()) {
                return font->second;
            }
            return nullptr;
        }

    private:
        void NewFrame();
        void EndFrame();
        void RenderFrame();

        void PrepareFontTexture();
        void CreateFontTextureDescriptors();
        void CreateGraphicsPipeline();

    private:
        Scope<VulkanImage> m_FontImage;
        UnorderedMap<String, Raw<ImFont>> m_Fonts;
        Ref<VulkanGraphicsPipeline> m_Pipeline;
    };
}