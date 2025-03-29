#include "services/renderer/DebugUIManager.hpp"
#include "services/renderer/PresentationRenderer.hpp"
#include "services/renderer/VulkanManager.hpp"
#include "services/assetmanager/AssetManager.hpp"
#include "services/CacheManager.hpp"
#include "core/Window.hpp"

#include <regex>

#include "imgui.h"
#include "imgui_impl_glfw.h"


namespace tlc {

    struct ImGuiVertex {
        ImDrawVert data = {};

        inline static List<vk::VertexInputAttributeDescription> GetAttributeDescriptions()
		{
			List<vk::VertexInputAttributeDescription> attributeDescriptions;
			attributeDescriptions.resize(3);

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = vk::Format::eR32G32Sfloat;
			attributeDescriptions[0].offset = offsetof(ImDrawVert, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = vk::Format::eR32G32Sfloat;
			attributeDescriptions[1].offset = offsetof(ImDrawVert, uv);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = vk::Format::eR8G8B8A8Unorm;
			attributeDescriptions[2].offset = offsetof(ImDrawVert, col);

			return attributeDescriptions;
		}
    };
    
    struct ImGuiPushConstants {
        glm::vec2 scale;
        glm::vec2 translate;
    };

    void DebugUIManager::Setup() {

    }   
    
    void DebugUIManager::OnStart() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        io.DisplayFramebufferScale = ImVec2(1.0, 1.0);


        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForVulkan(Window::Get()->GetHandle(), true);

        PrepareFontTexture();
        CreateFontTextureDescriptors();
        CreateGraphicsPipeline();
        CreateBuffers();
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
                auto dataCloned = (U8*)IM_ALLOC(fontDataSize);
                memcpy(dataCloned, fontAsset, fontDataSize);
                // Transfers the ownership of the font data
                auto fontPtr = io.Fonts->AddFontFromMemoryTTF(dataCloned, static_cast<I32>(fontDataSize), fontSize);
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

        m_FontDescriptorSet = descriptorSet;
        m_FontDescriptorSetLayout = descriptorSetLayout;
    }

    void DebugUIManager::CreateGraphicsPipeline() {
        auto presentationRenderer = Services::Get<PresentationRenderer>();
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
            .SetRenderPass(presentationRenderer->GetRenderPass())
            .SetExtent(vk::Extent2D()
                .setHeight(100)
                .setWidth(100))
            .SetVertexInputAttributeDescriptions<ImGuiVertex>()
            .AddPushConstantRange(vk::ShaderStageFlagBits::eVertex, 0, sizeof(ImGuiPushConstants))
            .AddDescriptorSetLayout(m_FontDescriptorSetLayout)
            .SetVertexShaderModule(vertShaderModule)
            .SetFragmentShaderModule(fragShaderModule);

        m_Pipeline = CreateRef<VulkanGraphicsPipeline>(device, pipelineSettings);
    }

    void DebugUIManager::CreateBuffers() {
        auto vulkan = Services::Get<VulkanManager>();
        auto device = vulkan->GetDevice();

        m_VertexBuffer = VulkanBuffer::CreateVertexBuffer(device, 1024);
        m_IndexBuffer = VulkanBuffer::CreateIndexBuffer(device, 1024);

        m_VertexStagingBuffer = VulkanBuffer::CreateStagingBuffer(device, 1024);
        m_IndexStagingBuffer = VulkanBuffer::CreateStagingBuffer(device, 1024);
    }

    void DebugUIManager::OnEnd() {

        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        
        m_FontImage.reset();

        m_VertexBuffer.reset();
        m_IndexBuffer.reset();
        m_VertexStagingBuffer.reset();
        m_IndexStagingBuffer.reset();
        m_Pipeline.reset();
    }

    void DebugUIManager::OnSceneChange() {

    }

    void DebugUIManager::OnEvent(const String& event, const String& eventParams) {
        
    }

    void DebugUIManager::NewFrame(U32 displayWidth, U32 displayHeight, F32 deltaTime) {
        auto& io = ImGui::GetIO();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }
    
    void DebugUIManager::EndFrame() {
        ImGui::EndFrame();
        ImGui::Render();
    }
    
    void DebugUIManager::RenderFrame(vk::CommandBuffer& commandBuffer, F32 deltaTime, U32 displayWidth, U32 displayHeight) {
        (void)deltaTime;

        auto imDrawData = ImGui::GetDrawData();
        auto& io = ImGui::GetIO();

        auto vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
        auto indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

        if (vertexBufferSize > 0 && indexBufferSize > 0) {
            m_VertexBuffer->ResizeToAtleast(vertexBufferSize);
            m_VertexStagingBuffer->ResizeToAtleast(vertexBufferSize);

            m_IndexBuffer->ResizeToAtleast(indexBufferSize);
            m_IndexStagingBuffer->ResizeToAtleast(indexBufferSize);

            static ImDrawVert s_vtxDst[1024 * 1024];
            static ImDrawIdx s_idxDst[1024 * 1024];

            auto vtxDst = s_vtxDst;
            auto idxDst = s_idxDst;

            for (auto i = 0 ; i < imDrawData->CmdListsCount ; i++) {
                const ImDrawList* lst = imDrawData->CmdLists[i];
                
                memcpy(vtxDst, lst->VtxBuffer.Data, lst->VtxBuffer.Size * sizeof(ImDrawVert));
                memcpy(idxDst, lst->IdxBuffer.Data, lst->IdxBuffer.Size * sizeof(ImDrawIdx));

                vtxDst += lst->VtxBuffer.Size;
                idxDst += lst->IdxBuffer.Size;
            }

            auto vertexBufferUploadSettings = VulkanBufferUploadSettings(s_vtxDst, vertexBufferSize)
                .SetStagingBuffer(m_VertexStagingBuffer);
            m_VertexBuffer->UploadSync(vertexBufferUploadSettings);

            auto indexBufferUploadSettings = VulkanBufferUploadSettings(s_idxDst, indexBufferSize)
                .SetStagingBuffer(m_IndexStagingBuffer);
            m_IndexBuffer->UploadSync(indexBufferUploadSettings);
        }

        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipeline->GetPipeline());

        auto pushConstants = ImGuiPushConstants {
            .scale = glm::vec2(2.0f / static_cast<F32>(io.DisplaySize.x), 2.0f / static_cast<F32>(io.DisplaySize.y)),
            .translate = glm::vec2(-1.0)
        };
        commandBuffer.pushConstants(m_Pipeline->GetPipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(ImGuiPushConstants), &pushConstants);

        auto viewport = vk::Viewport()
            .setWidth(io.DisplaySize.x)
            .setHeight(io.DisplaySize.y);
        commandBuffer.setViewport(0, { viewport });


        auto vertexOffset = 0;
		auto indexOffset = 0;

		if (imDrawData->CmdListsCount > 0) {
            commandBuffer.bindVertexBuffers(0, {m_VertexBuffer->GetBuffer()}, { 0 });
            commandBuffer.bindIndexBuffer(m_IndexBuffer->GetBuffer(), 0, vk::IndexType::eUint16);

			for (auto i = 0; i < imDrawData->CmdListsCount; i++)
			{
				const auto lst = imDrawData->CmdLists[i];
				for (auto j = 0; j < lst->CmdBuffer.Size; j++)
				{
					auto pcmd = &lst->CmdBuffer[j];

                    auto scissorRect = vk::Rect2D()
                        .setOffset(
                            vk::Offset2D()
                                .setX(std::max((I32)(pcmd->ClipRect.x), 0))
                                .setY(std::max((I32)(pcmd->ClipRect.y), 0)))
                        .setExtent(
                            vk::Extent2D()
                                .setWidth((U32)(pcmd->ClipRect.z - pcmd->ClipRect.x))
                                .setHeight((U32)(pcmd->ClipRect.w - pcmd->ClipRect.y)));
                    
                    auto descriptorSet = (VkDescriptorSet)pcmd->GetTexID();                                            
                    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_Pipeline->GetPipelineLayout(), 0, {descriptorSet}, {});

                    commandBuffer.setScissor(0, {scissorRect});
                    commandBuffer.drawIndexed(pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);

					indexOffset += pcmd->ElemCount;
				}
				vertexOffset += lst->VtxBuffer.Size;
			}
		}


    }

}