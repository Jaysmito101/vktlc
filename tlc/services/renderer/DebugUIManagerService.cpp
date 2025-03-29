#include "services/renderer/DebugUIManager.hpp"
#include "services/renderer/VulkanManager.hpp"
#include "services/assetmanager/AssetManager.hpp"
#include "services/CacheManager.hpp"
#include "core/Window.hpp"

#include <regex>

#include "imgui.h"
#include "imgui_impl_glfw.h"


namespace tlc {

    struct ImGuiVertex {
        ImVec2 pos = ImVec2(0.0, 0.0);
        ImVec2 uv = ImVec2(0.0, 0.0);
        ImColor color = ImColor(0);

        inline static List<vk::VertexInputAttributeDescription> GetAttributeDescriptions()
		{
			List<vk::VertexInputAttributeDescription> attributeDescriptions;
			attributeDescriptions.resize(3);

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = vk::Format::eR32G32Sfloat;
			attributeDescriptions[0].offset = offsetof(ImGuiVertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = vk::Format::eR32G32Sfloat;
			attributeDescriptions[1].offset = offsetof(ImGuiVertex, uv);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = vk::Format::eR8G8B8A8Unorm;
			attributeDescriptions[2].offset = offsetof(ImGuiVertex, color);

			return attributeDescriptions;
		}
    };

    void DebugUIManager::Setup() {

    }   
    
    void DebugUIManager::OnStart() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForVulkan(Window::Get()->GetHandle(), true);

        PrepareFontTexture();
        CreateFontTextureDescriptors();
    }

    void DebugUIManager::PrepareFontTexture() {
        auto assetManager = Services::Get<AssetManager>();
        auto vulkan = Services::Get<VulkanManager>();
        auto device = vulkan->GetDevice();
        
        auto& io = ImGui::GetIO(); (void)io;

            
        auto fonts = assetManager->GetAssetsWithTagsInBundle(AssetTags::Font, "debug");
        m_Fonts.insert_or_assign("default", io.Fonts->AddFontDefault());
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
                auto fontPtr = io.Fonts->AddFontFromMemoryTTF(fontAsset, static_cast<I32>(fontDataSize), fontSize);
                m_Fonts.insert_or_assign(font, fontPtr);
            }
        }
        if(!io.Fonts->Build()) {
            log::Error("Failed to build font atlas");
        }


        U8* fontAtlasData = nullptr;
        I32 fontAtlasWidth = 0;
        I32 fontAtlasHeight = 0;
        io.Fonts->GetTexDataAsRGBA32(&fontAtlasData, &fontAtlasWidth, &fontAtlasHeight);


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

        log::Info("Font image size: {}x{}", fontAtlasWidth, fontAtlasHeight);

        auto imageUploadSettings = VulkanImageUploadSettings(fontAtlasData, fontAtlasWidth, fontAtlasHeight, 4);
        if(!m_FontImage->UploadSync(imageUploadSettings)) {
            log::Error("Failed to upload font image data to Vulkan image");
        }
    }

    void DebugUIManager::CreateFontTextureDescriptors() {
        auto vulkan = Services::Get<VulkanManager>();
        auto device = vulkan->GetDevice();
        
        auto& io = ImGui::GetIO(); (void)io;


        auto descriptorSetLayoutBindings = {
            vk::DescriptorSetLayoutBinding()
                .setBinding(0)
                .setDescriptorCount(1)
                .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                .setStageFlags(vk::ShaderStageFlagBits::eFragment)
        };
        auto descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
            .setBindingCount(1)
            .setBindings(descriptorSetLayoutBindings);
        auto descriptorSetLayout = device->CreateDescriptorSetLayout(descriptorSetLayoutCreateInfo);
        auto descriptorSet = device->AllocateDescriptorSets("debug/imgui", vk::DescriptorType::eCombinedImageSampler, {descriptorSetLayout})[0];

        

        auto descriptorImageInfo = vk::DescriptorImageInfo()
            .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
            .setImageView(m_FontImage->GetImageView("FontImageView"))
            .setSampler(m_FontImage->GetSampler("FontImageSampler"));
        
        auto writeDescriptorSets = {
            vk::WriteDescriptorSet()
                .setDescriptorCount(1)
                .setDstSet(descriptorSet)
                .setDstBinding(0)
                .setImageInfo(descriptorImageInfo)
                .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
        };
        device->GetDevice().updateDescriptorSets(writeDescriptorSets, {});

        io.Fonts->SetTexID((ImTextureID)(void*)descriptorSet);
    }

    void DebugUIManager::CreateGraphicsPipeline() {
        auto cacheManager = Services::Get<CacheManager>();
        auto vulkan = Services::Get<VulkanManager>();
        auto device = vulkan->GetDevice();

        auto vertShaderModule = device->CreateShaderModule(
            cacheManager->GetCacheDataTyped<U32>("shaders/imgui/ui.vert.glsl")
        );

        auto fragShaderModule = device->CreateShaderModule(
            cacheManager->GetCacheDataTyped<U32>("shaders/imgui/ui.frag.glsl")
        );

        auto pipelineSettings = VulkanGraphicsPipelineSettings()
            .SetExtent(vk::Extent2D()
                .setHeight(100)
                .setWidth(100))
            .SetVertexInputAttributeDescriptions<ImGuiVertex>()
            .SetVertexShaderModule(vertShaderModule)
            .SetFragmentShaderModule(fragShaderModule);

        m_Pipeline = CreateRef<VulkanGraphicsPipeline>(device, pipelineSettings);
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