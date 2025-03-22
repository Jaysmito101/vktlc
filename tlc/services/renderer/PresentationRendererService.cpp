#include "services/renderer/PresentationRenderer.hpp"

namespace tlc {
    
    void PresentationRenderer::Setup() {

    }

    void PresentationRenderer::OnStart() {
        CreateSynchronizationObjects();
        CreateRenderPass();
    }

    void PresentationRenderer::OnEnd() {
        auto vulkan = Services::Get<VulkanManager>();
        auto device = vulkan->GetDevice();

        device->WaitIdle();
        DestroySynchronizationObjects();
        DestroyRenderPass();
    }
    
    void PresentationRenderer::OnSceneChange() {

    }
    
    void PresentationRenderer::OnEvent(const String& event, const String& eventParams) {
        
    }

    void PresentationRenderer::BeginRenderingCurrentFrame() {
        auto vulkan = Services::Get<VulkanManager>();
        auto device = vulkan->GetDevice();
        auto swapchain = vulkan->GetSwapchain();

        if (m_NumInflightFrames > vulkan->GetSwapchain()->GetImageCount()) {
            log::Warn("Number of inflight frames is greater than swapchain image count. Recreating render resources.");
            RecreateRenderResources();
            return;
        }
        
        VkCall(device->GetDevice().waitForFences({m_InFlightFences[m_CurrentFrameIndex]}, VK_TRUE, UINT64_MAX));
        device->GetDevice().resetFences({m_InFlightFences[m_CurrentFrameIndex]});

        auto imageIndex = swapchain->AcquireNextImage(m_ImageAvailableSemaphores[m_CurrentFrameIndex]);
        if (imageIndex.IsOk()) {
            m_CurrentImageIndex = *imageIndex;
        } else {
            auto error = imageIndex.Error();
            if (error == vk::Result::eErrorOutOfDateKHR || error == vk::Result::eSuboptimalKHR) {
                log::Warn("Swapchain is out of date or suboptimal. Recreating swapchain, framebuffers and render resources.");
                swapchain->Recreate(); // recreate the swapchain
                RecreateRenderResources();
                device->GetDevice().resetFences({m_InFlightFences[m_CurrentFrameIndex]}); // reset the inflight fence
                m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % m_NumInflightFrames; // increment the current frame index
                return;
            } else {
                VkCritCall(error);
            }
        }
    }

    void PresentationRenderer::EndRenderingCurrentFrame() {
        auto vulkan = Services::Get<VulkanManager>();
        auto device = vulkan->GetDevice();
        auto swapchain = vulkan->GetSwapchain();
        
        swapchain->PresentImage(m_CurrentImageIndex, m_RenderFinishedSemaphores[m_CurrentFrameIndex]);
        m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % m_NumInflightFrames;
    }
    

    void PresentationRenderer::CreateRenderPass() {
        auto vulkan = Services::Get<VulkanManager>();
        auto device = vulkan->GetDevice();
        auto swapchain = vulkan->GetSwapchain();
        
        Array<vk::AttachmentDescription, 1> attachments;

        attachments[0].format = swapchain->GetSurfaceFormat().format;
        attachments[0].samples = vk::SampleCountFlagBits::e1;
        attachments[0].loadOp = vk::AttachmentLoadOp::eClear;
        attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
        attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        attachments[0].initialLayout = vk::ImageLayout::eUndefined;
        attachments[0].finalLayout = vk::ImageLayout::ePresentSrcKHR;

        vk::AttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::SubpassDescription subpass = {};
        subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpass.colorAttachmentCount = 1;
        subpass.preserveAttachmentCount = 0;
        subpass.inputAttachmentCount = 0;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pInputAttachments = nullptr;
        subpass.pResolveAttachments = nullptr;
        subpass.pDepthStencilAttachment = nullptr;
        subpass.pPreserveAttachments = nullptr;

        Array<vk::SubpassDependency, 1> dependencies = {};
        
        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependencies[0].dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependencies[0].srcAccessMask = vk::AccessFlagBits::eNone;
        dependencies[0].dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead;

        vk::RenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = vk::StructureType::eRenderPassCreateInfo;
        renderPassInfo.attachmentCount = static_cast<U32>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = static_cast<U32>(dependencies.size());
        renderPassInfo.pDependencies = dependencies.data();
        renderPassInfo.pNext = nullptr;

        VkCritCall(device->GetDevice().createRenderPass(&renderPassInfo, nullptr, &m_RenderPass));
    }

	void PresentationRenderer::DestroyRenderPass() {
        auto vulkan = Services::Get<VulkanManager>();
        auto device = vulkan->GetDevice();
        if (m_RenderPass != VK_NULL_HANDLE) {
            device->GetDevice().destroyRenderPass(m_RenderPass);
            m_RenderPass = VK_NULL_HANDLE;
        }
    }

    void PresentationRenderer::CreateSynchronizationObjects() {
        auto vulkan = Services::Get<VulkanManager>();
        auto device = vulkan->GetDevice();
        m_NumInflightFrames = std::clamp(vulkan->GetSwapchain()->GetImageCount(), (Size)1, (Size)2);
        for (Size i = 0; i < m_NumInflightFrames; i++)
        {
            m_ImageAvailableSemaphores.push_back(device->CreateVkSemaphore());
            m_RenderFinishedSemaphores.push_back(device->CreateVkSemaphore());
            m_InFlightFences.push_back(device->CreateVkFence());
        }
        m_CurrentFrameIndex = 0;
    }

    void PresentationRenderer::DestroySynchronizationObjects() {
        auto vulkan = Services::Get<VulkanManager>();
        auto device = vulkan->GetDevice();
        for (Size i = 0; i < m_NumInflightFrames; i++)
        {
            device->DestroyVkSemaphore(m_ImageAvailableSemaphores[i]);
            device->DestroyVkSemaphore(m_RenderFinishedSemaphores[i]);
            device->DestroyVkFence(m_InFlightFences[i]);
        }
        m_ImageAvailableSemaphores.clear();
        m_RenderFinishedSemaphores.clear();
        m_InFlightFences.clear();
    }


    void PresentationRenderer::RecreateRenderResources() {
        auto vulkan = Services::Get<VulkanManager>();
        auto device = vulkan->GetDevice();
        DestroySynchronizationObjects();
        CreateSynchronizationObjects();

    }
}