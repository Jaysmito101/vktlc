#include "vulkanapi/VulkanImage.hpp"

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
        m_Device->GetDevice().bindImageMemory(m_Image, m_Memory, 0);

        m_IsReady = true;
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
            .setInitialLayout(m_Settings.initialLayout);

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
}

