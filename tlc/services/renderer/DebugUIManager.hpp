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

        inline Bool IsEditorOpen(const String& name) {
            auto editor = m_IsEditorOpen.find(name);
            if (editor != m_IsEditorOpen.end()) {
                return editor->second;
            }
            m_IsEditorOpen[name] = false;
            return false;
        }

        inline Raw<Bool> EditorOpenPtr(const String& name) {
            auto [iterator, inserted] = m_IsEditorOpen.try_emplace(name, false);
            return &iterator->second;
        }

        void NewFrame(U32 displayWidth, U32 displayHeight, F32 deltaTime);
        void BeginEditorSection();
        void EndEditorSection();
        void EndFrame();

        inline void ToggleDebugUI() { auto ptr = EditorOpenPtr("Main/DebugUI"); *ptr = !*ptr; }
        inline Bool IsDebugUIVisible() { return IsEditorOpen("Main/DebugUI"); }


    private:
        void UpdateBuffersIfNeeded(vk::CommandBuffer& commandBuffer, ImDrawData* imDrawData);
        void RenderFrame(vk::CommandBuffer& commandBuffer, F32 deltaTime, U32 displayWidth, U32 displayHeight);

        void PrepareFontTexture();
        void CreateFontTextureDescriptors();
        void CreateGraphicsPipeline();
        void CreateBuffers();

        friend class PresentationRenderer;

    private:
        Scope<VulkanImage> m_FontImage;

        vk::DescriptorSet m_FontDescriptorSet;
        vk::DescriptorSetLayout m_FontDescriptorSetLayout;

        UnorderedMap<String, Bool> m_IsEditorOpen;

        UnorderedMap<String, Raw<ImFont>> m_Fonts;
        Ref<VulkanGraphicsPipeline> m_Pipeline;
        Ref<VulkanBuffer> m_VertexBuffer;
        Ref<VulkanBuffer> m_IndexBuffer;
        Ref<VulkanBuffer> m_VertexStagingBuffer;
        Ref<VulkanBuffer> m_IndexStagingBuffer;
    };
}