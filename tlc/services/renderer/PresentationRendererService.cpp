#include "services/renderer/PresentationRenderer.hpp"
#include "services/renderer/DebugUIManager.hpp"

#include "services/CacheManager.hpp"
#include "core/Window.hpp"

namespace tlc {

    struct PresentationPipelineConfig {
        F32 viewportWidth = 0.0f;
        F32 viewportHeight = 0.0f;
        F32 renderFrameWidth = 0.0f;
        F32 renderFrameHeight = 0.0f;        
    };
    
    void PresentationRenderer::Setup() {

    }

    void PresentationRenderer::OnStart() {
        CreateSynchronizationObjects();
        CreateRenderPass();
        CreateFramebuffers();
        CreateCommandBuffers();
        CreatePipeline();

    }

    void PresentationRenderer::OnEnd() {
        auto vulkan = Services::Get<VulkanManager>();
        auto device = vulkan->GetDevice();

        device->WaitIdle();

        DestroySynchronizationObjects();
        DestroyFramebuffers();
        DestroyCommandBuffers();
        DestroyRenderPass();

        m_Pipeline.reset();
    }
    
    void PresentationRenderer::OnSceneChange() {

    }
    
    void PresentationRenderer::OnEvent(const String& event, const String& eventParams) {
        
    }

    Bool PresentationRenderer::RenderCurrentFrame(F32 deltaTime) {
        auto debugUi = Services::Get<DebugUIManager>();
        auto vulkan = Services::Get<VulkanManager>();
        auto device = vulkan->GetDevice();
        auto swapchain = vulkan->GetSwapchain();


        // If the number of inflight frames is greater than the swapchain image count, recreate the render resources
        if (m_NumInflightFrames > vulkan->GetSwapchain()->GetImageCount()) {
            log::Warn("Number of inflight frames is greater than swapchain image count. Recreating render resources.");
            RecreateRenderResources();
            return false;
        }

        // if the size of the swapchain is different from the last size, recreate the render resources
        if (swapchain->GetExtent().width != m_LastWindowSize.first || swapchain->GetExtent().height != m_LastWindowSize.second) {
            log::Warn("Swapchain size has changed. Recreating render resources.");
            m_LastWindowSize = MakePair(swapchain->GetExtent().width, swapchain->GetExtent().height);
            RecreateRenderResources();
            return false;
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
                RecreateRenderResources();
                device->GetDevice().resetFences({m_InFlightFences[m_CurrentFrameIndex]}); // reset the inflight fence
                m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % m_NumInflightFrames; // increment the current frame index
                return false;
            } else {
                VkCritCall(error);
            }
        }

        const auto& swaphcainExtent = swapchain->GetExtent();
        static Array<vk::ClearValue, 1> clearValues = {
            vk::ClearValue()
            .setColor(vk::ClearColorValue(std::array<F32, 4>{0.2f, 0.2f, 0.2f, 1.0f}))
        };

        auto renderPassBeginInfo = vk::RenderPassBeginInfo()
            .setRenderPass(m_RenderPass)
            .setFramebuffer(m_Framebuffers[m_CurrentImageIndex])
            .setRenderArea(vk::Rect2D()
                .setOffset(vk::Offset2D(0, 0))
                .setExtent(swaphcainExtent))
            .setClearValueCount(static_cast<U32>(clearValues.size()))
            .setPClearValues(clearValues.data());

        m_CurrentCommandBuffer = m_CommandBuffers[m_CurrentFrameIndex];
        m_CurrentCommandBuffer.reset();

        VkCall(m_CurrentCommandBuffer.begin(vk::CommandBufferBeginInfo()
            .setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse)
            .setPInheritanceInfo(nullptr)));
        
        m_CurrentCommandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

        m_CurrentCommandBuffer.setViewport(0, vk::Viewport()
            .setX(0.0f)
            .setY(0.0f)
            .setWidth(static_cast<F32>(swaphcainExtent.width))
            .setHeight(static_cast<F32>(swaphcainExtent.height))
            .setMinDepth(0.0f)
            .setMaxDepth(1.0f));
        m_CurrentCommandBuffer.setScissor(0, vk::Rect2D()
            .setOffset(vk::Offset2D(0, 0))
            .setExtent(swaphcainExtent));

        const auto presentationConfig = PresentationPipelineConfig {
            .viewportWidth = static_cast<F32>(swaphcainExtent.width),
            .viewportHeight = static_cast<F32>(swaphcainExtent.height),
            .renderFrameWidth = static_cast<F32>(1000),
            .renderFrameHeight = static_cast<F32>(1000)
        };

        m_CurrentCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipeline->GetPipeline());
        m_CurrentCommandBuffer.pushConstants(m_Pipeline->GetPipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(PresentationPipelineConfig), &presentationConfig);
        m_CurrentCommandBuffer.draw(6, 1, 0, 0);


        debugUi->RenderFrame(m_CurrentCommandBuffer, deltaTime, swaphcainExtent.width, swaphcainExtent.height);

        m_CurrentCommandBuffer.endRenderPass();
        VkCall(m_CurrentCommandBuffer.end());

        auto waitStages = vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput);

        auto submitInfo = vk::SubmitInfo()
            .setCommandBufferCount(1)
            .setPCommandBuffers(&m_CurrentCommandBuffer)
            .setSignalSemaphoreCount(1)
            .setPSignalSemaphores(&m_RenderFinishedSemaphores[m_CurrentFrameIndex])
            .setWaitSemaphoreCount(1)
            .setPWaitSemaphores(&m_ImageAvailableSemaphores[m_CurrentFrameIndex])
            .setPWaitDstStageMask(&waitStages);

        VkCall(device->GetQueue(VulkanQueueType::Graphics).submit({submitInfo}, m_InFlightFences[m_CurrentFrameIndex]));


        auto result = swapchain->PresentImage(m_CurrentImageIndex, m_RenderFinishedSemaphores[m_CurrentFrameIndex]);
        if (result != vk::Result::eSuccess) {
            if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
                log::Warn("Swapchain is out of date or suboptimal. Recreating swapchain, framebuffers and render resources.");
                RecreateRenderResources();
            } else {
                VkCritCall(result);
            }
        }
        m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % m_NumInflightFrames;

        return true;
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
            m_InFlightFences.push_back(device->CreateVkFence(vk::FenceCreateFlagBits::eSignaled));
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
        auto [result, commandBuffers] = device->GetDevice().allocateCommandBuffers(commandBufferAllocateInfo);
        VkCritCall(result);
        m_CommandBuffers = commandBuffers;
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
        auto swapchain = vulkan->GetSwapchain();
        
        log::Warn("Recreating render resources.");

        auto size = Window::Get()->GetSize();

        if (size.first == 0 || size.second == 0) {
            log::Warn("Window size is 0. Not recreating render resources.");
            return;
        }
        
        swapchain->Recreate(); // recreate the swapchain
        device->WaitIdle();

        
        if (m_NumInflightFrames > vulkan->GetSwapchain()->GetImageCount()) {
            DestroySynchronizationObjects();
            CreateSynchronizationObjects();

            DestroyCommandBuffers();
            CreateCommandBuffers();
        }

        DestroyFramebuffers();
        CreateFramebuffers();

        // raise the event for the swapchain recreate
        EventManager<EventType::SwapchainRecreate, U32>::Get()->RaiseEvent(static_cast<U32>(m_NumInflightFrames)); 
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
            .SetExtent(vulkan->GetSwapchain()->GetExtent())
            .AddPushConstantRange(vk::ShaderStageFlagBits::eVertex, 0, sizeof(PresentationPipelineConfig));

        m_Pipeline = CreateScope<VulkanGraphicsPipeline>(
            device, 
            pipelineSettings
        );
    }
}