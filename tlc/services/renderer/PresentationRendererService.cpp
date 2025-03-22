#include "services/renderer/PresentationRenderer.hpp"

#include "services/CacheManager.hpp"

namespace tlc {
    
    void PresentationRenderer::Setup() {

    }

    void PresentationRenderer::OnStart() {
        CreateSynchronizationObjects();
        CreateRenderPass();
        CreateFramebuffers();
        CreatePipeline();
    }

    void PresentationRenderer::OnEnd() {
        auto vulkan = Services::Get<VulkanManager>();
        auto device = vulkan->GetDevice();

        device->WaitIdle();
        DestroySynchronizationObjects();
        DestroyFramebuffers();
        DestroyRenderPass();

        m_Pipeline.reset();
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
        
        Array<vk::AttachmentDescription, 1> attachments = {
            vk::AttachmentDescription()
            .setFormat(swapchain->GetSurfaceFormat().format)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::ePresentSrcKHR)
        };
            
        auto colorAttachmentRef = vk::AttachmentReference()
            .setLayout(vk::ImageLayout::eColorAttachmentOptimal)
            .setAttachment(0);

        auto subpass = vk::SubpassDescription()
            .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
            .setColorAttachmentCount(1)
            .setInputAttachmentCount(0)
            .setPColorAttachments(&colorAttachmentRef)
            .setPInputAttachments(nullptr)
            .setPResolveAttachments(nullptr)
            .setPDepthStencilAttachment(nullptr)
            .setPPreserveAttachments(nullptr);
            
        
        Array<vk::SubpassDependency, 1> dependencies = {
            vk::SubpassDependency()
            .setSrcSubpass(VK_SUBPASS_EXTERNAL)
            .setDstSubpass(0)
            .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setSrcAccessMask(vk::AccessFlagBits::eNone)
            .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead)
        };
        

        auto renderPassInfo = vk::RenderPassCreateInfo()
            .setAttachmentCount(static_cast<U32>(attachments.size()))
            .setPAttachments(attachments.data())
            .setSubpassCount(1)
            .setPSubpasses(&subpass)
            .setDependencyCount(static_cast<U32>(dependencies.size()))
            .setPDependencies(dependencies.data())
            .setPNext(nullptr);

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

    void PresentationRenderer::CreateFramebuffers() {
        auto vulkan = Services::Get<VulkanManager>();
        auto device = vulkan->GetDevice();
        auto swapchain = vulkan->GetSwapchain();

        m_Framebuffers = swapchain->CreateFramebuffers(m_RenderPass);
    }
    
    void PresentationRenderer::DestroyFramebuffers() {
        auto vulkan = Services::Get<VulkanManager>();
        auto device = vulkan->GetDevice();
        for (auto framebuffer : m_Framebuffers) {
            device->GetDevice().destroyFramebuffer(framebuffer);
        }
        m_Framebuffers.clear();
    }

    
    void PresentationRenderer::CreateCommandBuffers() {
        auto vulkan = Services::Get<VulkanManager>();
        auto device = vulkan->GetDevice();
        auto swapchain = vulkan->GetSwapchain();

        auto commandBufferAllocateInfo = vk::CommandBufferAllocateInfo()
            .setCommandPool(device->GetCommandPool(VulkanQueueType::Graphics))
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount(static_cast<U32>(m_NumInflightFrames));

        m_CommandBuffers.resize(m_NumInflightFrames);
        m_CommandBuffers = device->GetDevice().allocateCommandBuffers(commandBufferAllocateInfo);
    }

    void PresentationRenderer::DestroyCommandBuffers() {
        auto vulkan = Services::Get<VulkanManager>();
        auto device = vulkan->GetDevice();
        device->GetDevice().freeCommandBuffers(device->GetCommandPool(VulkanQueueType::Graphics), m_CommandBuffers);
        m_CommandBuffers.clear();
        m_CurrentCommandBuffer = VK_NULL_HANDLE;
    }

    void PresentationRenderer::RecreateRenderResources() {
        auto vulkan = Services::Get<VulkanManager>();
        auto device = vulkan->GetDevice();

        // TODO: This is not necessary for everytime RecreateRenderResources is called
        DestroySynchronizationObjects();
        CreateSynchronizationObjects();

        // TODO: This is not necessary for everytime RecreateRenderResources is called
        DestroyCommandBuffers();
        CreateCommandBuffers();

        DestroyFramebuffers();
        CreateFramebuffers();

        m_Pipeline->GetSettings().SetExtent(vulkan->GetSwapchain()->GetExtent());
        m_Pipeline->Recreate();
    }


    void PresentationRenderer::CreatePipeline() {
        auto cacheManager = Services::Get<CacheManager>();
        auto vulkan = Services::Get<VulkanManager>();
        auto device = vulkan->GetDevice();

        auto vertShaderModule = device->CreateShaderModule(
            cacheManager->GetCacheDataTyped<U32>("shaders/presentation/vert.glsl")
        );
        auto fragShaderModule = device->CreateShaderModule(
            cacheManager->GetCacheDataTyped<U32>("shaders/presentation/frag.glsl")
        );  

        auto pipelineSettings = VulkanGraphicsPipelineSettings()
            .SetRenderPass(m_RenderPass)
            .SetVertexShaderModule(vertShaderModule)
            .SetFragmentShaderModule(fragShaderModule)
            .SetExtent(vulkan->GetSwapchain()->GetExtent());

        m_Pipeline = CreateScope<VulkanGraphicsPipeline>(
            device, 
            pipelineSettings
        );
    }
}