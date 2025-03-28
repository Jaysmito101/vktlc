#include "services/renderer/DebugUIManager.hpp"
#include "services/renderer/VulkanManager.hpp"
#include "services/assetmanager/AssetManager.hpp"
#include "core/Window.hpp"

#include <regex>

#include "imgui.h"
#include "imgui_impl_glfw.h"


namespace tlc {

    void DebugUIManager::Setup() {

    }   
    
    void DebugUIManager::OnStart() {
        auto assetManager = Services::Get<AssetManager>();
        auto vulkan = Services::Get<VulkanManager>();
        auto device = vulkan->GetDevice();


        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

        
        auto fonts = assetManager->GetAssetsWithTagsInBundle(AssetTags::Font, "debug");
        for (const auto& font : fonts) {
            Size fontDataSize = 0;
            auto fontAsset = assetManager->GetAssetDataRaw(font, fontDataSize);
            if (fontAsset != nullptr) {
                // check if there is a number(in the format xx.x) in the font name if so use it as the font size
                // using regex to find the number
                std::regex regex(R"(\d+(\.\d+)?)");
                std::smatch match;
                auto fontSize = 16.0f;
                if (std::regex_search(font, match, regex)) {
                    fontSize = std::stof(match.str(0));
                }
                io.Fonts->AddFontFromMemoryTTF(fontAsset, fontDataSize, fontSize);
            }
        }

        
        U8* fontAtlasData = nullptr;
        I32 fontAtlasWidth = 0;
        I32 fontAtlasHeight = 0;
        io.Fonts->GetTexDataAsRGBA32(&fontAtlasData, &fontAtlasWidth, &fontAtlasHeight);
        
        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForVulkan(Window::Get()->GetHandle(), true);

        auto vulkanImageSettings = VulkanImageSettings()
            .SetSize(fontAtlasWidth, fontAtlasHeight)
            .SetFormat(vk::Format::eR8G8B8A8Unorm)
            .SetUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled);
        m_FontImage = CreateScope<VulkanImage>(device, vulkanImageSettings);
        m_FontImage->CreateView("FontImageView", vk::ImageViewType::e2D, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor, 0, 1);

        auto samplerSettings = VulkanSamplerSettings()
            .SetUAddressMode(vk::SamplerAddressMode::eClampToEdge)
            .SetVAddressMode(vk::SamplerAddressMode::eClampToEdge)
            .SetWAddressMode(vk::SamplerAddressMode::eClampToEdge)
            .SetBorderColor(vk::BorderColor::eFloatOpaqueWhite)
            .SetMipmapMode(vk::SamplerMipmapMode::eLinear);
        m_FontImage->CreateSampler("FontImageSampler", samplerSettings);
    }

    void DebugUIManager::OnEnd() {

        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        
        m_FontImage.reset();
    }

    void DebugUIManager::OnSceneChange() {

    }

    void DebugUIManager::OnEvent(const String& event, const String& eventParams) {
        
    }

    void DebugUIManager::NewFrame() {
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }
    
    void DebugUIManager::EndFrame() {
        ImGui::EndFrame();
        ImGui::Render();
    }
    
    void DebugUIManager::RenderFrame() {
    }

}