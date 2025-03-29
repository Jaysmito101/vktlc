#include "vulkanapi/VulkanImage.hpp"

#include "vulkanapi/VulkanBuffer.hpp"
#include "vulkanapi/VulkanDevice.hpp"

namespace tlc {


    VulkanImage::VulkanImage(Raw<VulkanDevice> device, const VulkanImageSettings& settings)
    : m_Device(device), m_Settings(settings)
    {
        if (!Recreate())
        {
            log::Warn("Failed to create Vulkan image");
        }
    }


    VulkanImage::~VulkanImage()
    {
        if (m_IsReady) {
            Cleanup();
        }
    }

    Bool VulkanImage::Recreate()
    {
        if (m_IsReady) {
            Cleanup();
        }

        auto imageCreateInfo = GetCreateInfo();
        auto [result, image] =  m_Device->GetDevice().createImage(imageCreateInfo);;
        if (result != vk::Result::eSuccess) {
            VkCall(result);
            return false;
        }
        m_Image = image;
        
        auto memoryRequirements = m_Device->GetDevice().getImageMemoryRequirements(m_Image);

        if (m_Settings.useDeviceMemoryPool) {
            log::Fatal("Device Memory Pool has not yet been implemented!");
        } else {
            auto memoryAllocationInfo = vk::MemoryAllocateInfo()
                .setAllocationSize(memoryRequirements.size)
                .setMemoryTypeIndex(m_Device->FindMemoryType(memoryRequirements.memoryTypeBits, m_Settings.memoryPropertyFlags));
            auto [result, memory] = m_Device->GetDevice().allocateMemory(memoryAllocationInfo);
            if (result != vk::Result::eSuccess) {
                VkCall(result);
                return false;
            }
            m_Memory = memory;            
        }
        VkCall(m_Device->GetDevice().bindImageMemory(m_Image, m_Memory, 0));

        TransitionImageLayout(
            vk::ImageLayout::eUndefined,
            m_Settings.targetLayout,
            vk::ImageSubresourceRange()
                .setAspectMask(m_Settings.aspectMask)
                .setBaseMipLevel(0)
                .setLevelCount(m_Settings.mipLevels)
                .setBaseArrayLayer(0)
                .setLayerCount(m_Settings.arrayLayers),
            vk::PipelineStageFlagBits::eAllCommands,
            vk::PipelineStageFlagBits::eAllCommands
        );

        m_CurrentLayout = m_Settings.targetLayout;

        m_IsReady = true;

        return true;
    }

    void VulkanImage::Cleanup() {
        if(!m_IsReady) {
            log::Trace("Image is not ready! Cannot cleanup.");
            return;
        }

        m_Device->WaitIdle();

        m_Device->GetDevice().destroyImage(m_Image);

        for (const auto& [_, view] : m_ImageViews) {
            m_Device->GetDevice().destroyImageView(view);
        }
        m_ImageViews.clear();

        for (const auto& [_, sampler] : m_Samplers) {
            m_Device->GetDevice().destroySampler(sampler);
        }
        m_Samplers.clear();

        if (m_Settings.useDeviceMemoryPool) {
            log::Fatal("Device Memory Pool has not yet been implemented!");
        } else {
            m_Device->GetDevice().freeMemory(m_Memory);
        }


        m_IsReady = false;
    }

    vk::ImageCreateInfo VulkanImage::GetCreateInfo() {
        auto imageCreateInfo = vk::ImageCreateInfo()
            .setImageType(m_Settings.imageType)
            .setFormat(m_Settings.format)
            .setExtent(vk::Extent3D()
                .setWidth(m_Settings.size.first)
                .setHeight(m_Settings.size.second)
                .setDepth(m_Settings.depth))
            .setMipLevels(m_Settings.mipLevels)
            .setArrayLayers(m_Settings.arrayLayers)
            .setSamples(m_Settings.samples)
            .setTiling(m_Settings.tiling)
            .setUsage(m_Settings.usage)
            .setSharingMode(m_Settings.sharingMode)
            .setInitialLayout(vk::ImageLayout::eUndefined);

        return imageCreateInfo;
    }

    Bool VulkanImage::CreateView(const String& name, vk::ImageViewType viewType, vk::Format format, vk::ImageAspectFlags aspectMask, U32 baseMipLevel, U32 levelCount) {
        if (m_ImageViews.contains(name)) {
            log::Warn("Image view with name {} already exists", name);
            return false;
        }

        auto imageViewCreateInfo = vk::ImageViewCreateInfo()
            .setFormat(format)
            .setImage(m_Image)
            .setViewType(viewType)
            .setSubresourceRange(vk::ImageSubresourceRange()
                .setAspectMask(aspectMask)
                .setBaseArrayLayer(0)
                .setLayerCount(m_Settings.arrayLayers)
                .setBaseMipLevel(baseMipLevel)
                .setLevelCount(levelCount));
            
        auto [result, view] = m_Device->GetDevice().createImageView(imageViewCreateInfo);
        if (result != vk::Result::eSuccess) {
            VkCall(result);
            return false;
        }

        m_ImageViews[name] = view;

        return true;
    }

    Bool VulkanImage::CreateSampler(const String& name, VulkanSamplerSettings settings) {
        if (m_Samplers.contains(name)) {
            log::Warn("Sampler with name {} already exists", name);
            return false;
        }

        // check for device support for anisotropy
        if (settings.anisotropyEnable) {
            auto features = m_Device->GetPhysicalDevice().getFeatures();
            if (!features.samplerAnisotropy) {
                log::Warn("Device does not support anisotropic filtering, disabling it");
                settings.anisotropyEnable = false;
            }

            auto properties = m_Device->GetPhysicalDevice().getProperties();
            if (settings.maxAnisotropy > properties.limits.maxSamplerAnisotropy) {
                log::Warn("Max anisotropy {} is greater than device limit {}, clamping to device limit", settings.maxAnisotropy, properties.limits.maxSamplerAnisotropy);
                settings.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
            }
        }

        auto samplerCreateInfo = vk::SamplerCreateInfo()
            .setMagFilter(settings.magFilter)
            .setMinFilter(settings.minFilter)
            .setAddressModeU(settings.uAddressMode)
            .setAddressModeV(settings.vAddressMode)
            .setAddressModeW(settings.wAddressMode)
            .setAnisotropyEnable(settings.anisotropyEnable)
            .setMaxAnisotropy(settings.maxAnisotropy)
            .setBorderColor(settings.borderColor)
            .setUnnormalizedCoordinates(settings.unnormalizedCoordinates)
            .setCompareEnable(settings.compareEnable)
            .setCompareOp(settings.compareOp)
            .setMinLod(settings.minLod)
            .setMaxLod(settings.maxLod)
            .setMipmapMode(settings.mipmapMode);

        auto [result, sampler] = m_Device->GetDevice().createSampler(samplerCreateInfo);
        if (result != vk::Result::eSuccess) {
            VkCall(result);
            return false;
        }

        m_Samplers[name] = sampler;

        return true;
    }

    VulkanImageAsyncUploadResult VulkanImage::UploadAsync(
        const VulkanImageUploadSettings& uploadSettings,
        vk::CommandBuffer commandBuffer
    ) {
        auto requiredSize = uploadSettings.GetRerquireImageSize();
        Ref<VulkanBuffer> stagingBuffer = nullptr;
        if (uploadSettings.stagingBuffer && uploadSettings.stagingBuffer->IsReady() && uploadSettings.stagingBuffer->GetSize() >= requiredSize) {
            stagingBuffer = uploadSettings.stagingBuffer;
        } else {
            stagingBuffer = VulkanBuffer::CreateStagingBuffer(m_Device, requiredSize);
        }

        if (!stagingBuffer->IsReady()) {
            return VulkanImageAsyncUploadResult::Faliure("Failed to create staging buffer for image upload");
        }

        if(!stagingBuffer->SetData(uploadSettings.data, requiredSize)) {
            return VulkanImageAsyncUploadResult::Faliure("Failed to set data for staging buffer");
        }

        auto transitionSubresporceRange = vk::ImageSubresourceRange()
            .setAspectMask(uploadSettings.aspectMask)
            .setBaseMipLevel(uploadSettings.mipLevel)
            .setLevelCount(1)
            .setBaseArrayLayer(uploadSettings.baseArrayLayer)
            .setLayerCount(uploadSettings.layerCount);

        TransitionImageLayoutWithCommandBuffer(
            commandBuffer,
            m_CurrentLayout,
            vk::ImageLayout::eTransferDstOptimal,
            transitionSubresporceRange,
            vk::PipelineStageFlagBits::eAllCommands,
            vk::PipelineStageFlagBits::eTransfer
        );

        auto bufferCopyRegion = vk::BufferImageCopy()
            .setBufferOffset(0)
            .setBufferRowLength(uploadSettings.size.first)
            .setBufferImageHeight(uploadSettings.size.second)
            .setImageSubresource(vk::ImageSubresourceLayers()
                .setAspectMask(uploadSettings.aspectMask)
                .setMipLevel(uploadSettings.mipLevel)
                .setBaseArrayLayer(uploadSettings.baseArrayLayer)
                .setLayerCount(uploadSettings.layerCount))
            .setImageOffset(vk::Offset3D()
                .setX(uploadSettings.offset.first)
                .setY(uploadSettings.offset.second)
                .setZ(uploadSettings.depthOffset))
            .setImageExtent(vk::Extent3D()
                .setWidth(uploadSettings.size.first)
                .setHeight(uploadSettings.size.second)
                .setDepth(uploadSettings.depth));

        commandBuffer.copyBufferToImage(
            stagingBuffer->GetBuffer(),
            m_Image,
            vk::ImageLayout::eTransferDstOptimal,
            { bufferCopyRegion }
        );

        TransitionImageLayoutWithCommandBuffer(
            commandBuffer,
            vk::ImageLayout::eTransferDstOptimal,
            m_CurrentLayout,
            transitionSubresporceRange,
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eComputeShader
        );

        auto result = VulkanImageAsyncUploadResult {
            .success = true,
            .stagingBuffer = stagingBuffer
        };
        
        return result;
    }

    Bool VulkanImage::UploadSync(const VulkanImageUploadSettings& uploadSettings) {
        auto commandBufferAllocateInfo = vk::CommandBufferAllocateInfo()
            .setCommandPool(m_Device->GetCommandPool(uploadSettings.queueType))
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount(1);

        auto [result, commandBuffers] = m_Device->GetDevice().allocateCommandBuffers(commandBufferAllocateInfo);
        if (result != vk::Result::eSuccess) {
            VkCall(result);
            return false;
        }
        auto commandBuffer = commandBuffers[0];

        auto commandBufferBeginInfo = vk::CommandBufferBeginInfo()
            .setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit)
            .setPInheritanceInfo(nullptr);
        VkCall(commandBuffer.begin(commandBufferBeginInfo));

        auto uploadResult = UploadAsync(uploadSettings, commandBuffer);

        VkCall(commandBuffer.end());

        auto submitInfo = vk::SubmitInfo()
            .setCommandBufferCount(1)
            .setPCommandBuffers(&commandBuffer);

        auto fence = m_Device->CreateVkFence(vk::FenceCreateFlagBits::eSignaled);
        m_Device->GetDevice().resetFences({fence});
        VkCall(m_Device->GetQueue(uploadSettings.queueType).submit({ submitInfo }, fence));
        VkCall(m_Device->GetDevice().waitForFences( { fence }, VK_TRUE, UINT64_MAX));

        m_Device->DestroyVkFence(fence);
        m_Device->GetDevice().freeCommandBuffers(m_Device->GetCommandPool(uploadSettings.queueType), { commandBuffer });

        return uploadResult.success;
    }

    Bool VulkanImage::TransitionImageLayout(
        vk::ImageLayout oldImageLayout,
        vk::ImageLayout newImageLayout,
        vk::ImageSubresourceRange subresourceRange,
        vk::PipelineStageFlags srcStageMask,
        vk::PipelineStageFlags dstStageMask,
        VulkanQueueType queueType
    ) {
        auto commandBufferAllocateInfo = vk::CommandBufferAllocateInfo()
            .setCommandPool(m_Device->GetCommandPool(VulkanQueueType::Graphics))
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount(1);
        auto [result, commandBuffers] = m_Device->GetDevice().allocateCommandBuffers(commandBufferAllocateInfo);
        if (result != vk::Result::eSuccess) {
            VkCall(result);
            return false;
        }
        auto commandBuffer = commandBuffers[0];

        auto commandBufferBeginInfo = vk::CommandBufferBeginInfo()
            .setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit)
            .setPInheritanceInfo(nullptr);
        VkCall(commandBuffer.begin(commandBufferBeginInfo));

        TransitionImageLayoutWithCommandBuffer(
            commandBuffer,
            oldImageLayout,
            newImageLayout,
            subresourceRange,
            srcStageMask,
            dstStageMask
        );

        VkCall(commandBuffer.end());

        auto submitInfo = vk::SubmitInfo()
            .setCommandBufferCount(1)
            .setPCommandBuffers(&commandBuffer);

        auto fence = m_Device->CreateVkFence(vk::FenceCreateFlagBits::eSignaled);
        m_Device->GetDevice().resetFences( { fence } );
        VkCall(m_Device->GetQueue(queueType).submit({ submitInfo }, fence));
        VkCall(m_Device->GetDevice().waitForFences( { fence }, VK_TRUE, UINT64_MAX));
        m_Device->DestroyVkFence(fence);
        m_Device->GetDevice().freeCommandBuffers(m_Device->GetCommandPool(VulkanQueueType::Graphics), { commandBuffer });
        return true;
    }

    // From: https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanTools.cpp
    Bool VulkanImage::TransitionImageLayoutWithCommandBuffer(
        vk::CommandBuffer commandBuffer,
        vk::ImageLayout oldImageLayout,
        vk::ImageLayout newImageLayout,
        vk::ImageSubresourceRange subresourceRange,
        vk::PipelineStageFlags srcStageMask,
        vk::PipelineStageFlags dstStageMask
    ) {
        auto imageMemoryBarrier = vk::ImageMemoryBarrier()
            .setOldLayout(oldImageLayout)
            .setNewLayout(newImageLayout)
            .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
            .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
            .setImage(m_Image)
            .setSubresourceRange(subresourceRange);

        	// Source layouts (old)
			// Source access mask controls actions that have to be finished on the old layout
			// before it will be transitioned to the new layout
			switch (oldImageLayout)
			{
			case vk::ImageLayout::eUndefined:
				// Image layout is undefined (or does not matter)
				// Only valid as initial layout
				// No flags required, listed only for completeness
				imageMemoryBarrier.setSrcAccessMask(vk::AccessFlagBits::eNone);
				break;

            case vk::ImageLayout::ePreinitialized:
				// Image is preinitialized
				// Only valid as initial layout for linear images, preserves memory contents
				// Make sure host writes have been finished
                imageMemoryBarrier.setSrcAccessMask(vk::AccessFlagBits::eHostWrite);
				break;

            case vk::ImageLayout::eColorAttachmentOptimal:
				// Image is a color attachment
				// Make sure any writes to the color buffer have been finished
                imageMemoryBarrier.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
				break;

            case vk::ImageLayout::eDepthStencilAttachmentOptimal:
				// Image is a depth/stencil attachment
				// Make sure any writes to the depth/stencil buffer have been finished
                imageMemoryBarrier.setSrcAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite);
				break;

            case vk::ImageLayout::eTransferSrcOptimal:
				// Image is a transfer source
				// Make sure any reads from the image have been finished
                imageMemoryBarrier.setSrcAccessMask(vk::AccessFlagBits::eTransferRead);
				break;

            case vk::ImageLayout::eTransferDstOptimal:
				// Image is a transfer destination
				// Make sure any writes to the image have been finished
                imageMemoryBarrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
				break;

            case vk::ImageLayout::eShaderReadOnlyOptimal:
				// Image is read by a shader
				// Make sure any shader reads from the image have been finished
                imageMemoryBarrier.setSrcAccessMask(vk::AccessFlagBits::eShaderRead);
				break;
			default:
				// Other source layouts aren't handled (yet)
				break;
			}

			// Target layouts (new)
			// Destination access mask controls the dependency for the new image layout
			switch (newImageLayout)
			{
            case vk::ImageLayout::eTransferDstOptimal:
				// Image will be used as a transfer destination
				// Make sure any writes to the image have been finished
                imageMemoryBarrier.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
				break;

            case vk::ImageLayout::eTransferSrcOptimal:
				// Image will be used as a transfer source
				// Make sure any reads from the image have been finished
                imageMemoryBarrier.setDstAccessMask(vk::AccessFlagBits::eTransferRead);
				break;

            case vk::ImageLayout::eColorAttachmentOptimal:
				// Image will be used as a color attachment
				// Make sure any writes to the color buffer have been finished
                imageMemoryBarrier.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
				break;

            case vk::ImageLayout::eDepthStencilAttachmentOptimal:
				// Image layout will be used as a depth/stencil attachment
				// Make sure any writes to depth/stencil buffer have been finished
                imageMemoryBarrier.setDstAccessMask(imageMemoryBarrier.dstAccessMask | vk::AccessFlagBits::eDepthStencilAttachmentWrite);
				break;

            case vk::ImageLayout::eShaderReadOnlyOptimal:
				// Image will be read in a shader (sampler, input attachment)
				// Make sure any writes to the image have been finished
				if (imageMemoryBarrier.srcAccessMask == vk::AccessFlagBits::eNone) {
                    imageMemoryBarrier.setSrcAccessMask(vk::AccessFlagBits::eHostWrite | vk::AccessFlagBits::eTransferWrite);
				}
                imageMemoryBarrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);
				break;
			default:
				// Other source layouts aren't handled (yet)
				break;
			}

        commandBuffer.pipelineBarrier(
            srcStageMask,
            dstStageMask,
            vk::DependencyFlags(),
            {},
            {},
            { imageMemoryBarrier }
        );

        return true;
    }

}

