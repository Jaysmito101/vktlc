#pragma once

#include "vulkanapi/VulkanBase.hpp"
#include "vulkanapi/VulkanDevice.hpp"

namespace tlc
{
	class VulkanContext;
	class VulkanDevice;

    struct VulkanImageSettings {
        vk::ImageType imageType = vk::ImageType::e2D;
        vk::Format format = vk::Format::eR8G8B8A8Unorm;
        Pair<U32, U32> size = MakePair(1, 1);
        U32 depth = 1;
        U32 mipLevels = 1;
        U32 arrayLayers = 1;
        vk::ImageAspectFlags aspectMask = vk::ImageAspectFlagBits::eColor;
        vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1;
        vk::ImageTiling tiling = vk::ImageTiling::eOptimal;
        vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst;
        vk::SharingMode sharingMode = vk::SharingMode::eExclusive;
        vk::ImageLayout targetLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        vk::MemoryPropertyFlags memoryPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
        // Raw<VulkanDeviceMemoryPool> deviceMemoryPool = nullptr; // TODO
        Bool useDeviceMemoryPool = false;
        


        inline VulkanImageSettings& SetImageType(vk::ImageType type) { imageType = type; return *this; }
        inline VulkanImageSettings& SetFormat(vk::Format fmt) { format = fmt; return *this; }
        inline VulkanImageSettings& SetSize(U32 width, U32 height) { size = MakePair(width, height); return *this; }
        inline VulkanImageSettings& SetDepth(U32 d) { depth = d; return *this; }
        inline VulkanImageSettings& SetMipLevels(U32 levels) { mipLevels = levels; return *this; }
        inline VulkanImageSettings& SetArrayLayers(U32 layers) { arrayLayers = layers; return *this; }
        inline VulkanImageSettings& SetSamples(vk::SampleCountFlagBits s) { samples = s; return *this; }
        inline VulkanImageSettings& SetTiling(vk::ImageTiling t) { tiling = t; return *this; }
        inline VulkanImageSettings& SetUsage(vk::ImageUsageFlags u) { usage = u; return *this; }
        inline VulkanImageSettings& SetSharingMode(vk::SharingMode mode) { sharingMode = mode; return *this; }
        inline VulkanImageSettings& SetTargetLayout(vk::ImageLayout layout) { targetLayout = layout; return *this; }
        inline VulkanImageSettings& SetMemoryPropertyFlags(vk::MemoryPropertyFlags flags) { memoryPropertyFlags = flags; return *this; }
        inline VulkanImageSettings& SetAspectMask(vk::ImageAspectFlags mask) { aspectMask = mask; return *this; }
        inline VulkanImageSettings& SetUseDeviceMemoryPool(Bool u) { useDeviceMemoryPool = u; return *this; }
        // inline VulkanImageSettings& SetDeviceMemoryPool(Raw<VulkanDeviceMemoryPool> p) { deviceMemoryPool = p; return *this; }
    };

    struct VulkanSamplerSettings {
        vk::Filter magFilter = vk::Filter::eLinear;
        vk::Filter minFilter = vk::Filter::eLinear;
        vk::SamplerAddressMode uAddressMode = vk::SamplerAddressMode::eRepeat;
        vk::SamplerAddressMode vAddressMode = vk::SamplerAddressMode::eRepeat;
        vk::SamplerAddressMode wAddressMode = vk::SamplerAddressMode::eRepeat;
        Bool anisotropyEnable = false;
        F32 maxAnisotropy = 1.0f;
        vk::BorderColor borderColor = vk::BorderColor::eIntOpaqueBlack;
        Bool unnormalizedCoordinates = false;
        Bool compareEnable = false;
        vk::CompareOp compareOp = vk::CompareOp::eAlways;
        F32 minLod = 0.0f;
        F32 maxLod = 0.0f;
        F32 mipLodBias = 0.0f;
        vk::SamplerMipmapMode mipmapMode = vk::SamplerMipmapMode::eLinear;

        inline VulkanSamplerSettings& SetMagFilter(vk::Filter filter) { magFilter = filter; return *this; }
        inline VulkanSamplerSettings& SetMinFilter(vk::Filter filter) { minFilter = filter; return *this; }
        inline VulkanSamplerSettings& SetUAddressMode(vk::SamplerAddressMode mode) { uAddressMode = mode; return *this; }
        inline VulkanSamplerSettings& SetVAddressMode(vk::SamplerAddressMode mode) { vAddressMode = mode; return *this; }
        inline VulkanSamplerSettings& SetWAddressMode(vk::SamplerAddressMode mode) { wAddressMode = mode; return *this; }
        inline VulkanSamplerSettings& SetAnisotropyEnable(Bool enable) { anisotropyEnable = enable; return *this; }
        inline VulkanSamplerSettings& SetMaxAnisotropy(F32 max) { maxAnisotropy = max; return *this; }
        inline VulkanSamplerSettings& SetBorderColor(vk::BorderColor color) { borderColor = color; return *this; }
        inline VulkanSamplerSettings& SetUnnormalizedCoordinates(Bool enable) { unnormalizedCoordinates = enable; return *this; }
        inline VulkanSamplerSettings& SetCompareEnable(Bool enable) { compareEnable = enable; return *this; }
        inline VulkanSamplerSettings& SetCompareOp(vk::CompareOp op) { compareOp = op; return *this; }
        inline VulkanSamplerSettings& SetMinLod(F32 lod) { minLod = lod; return *this; }
        inline VulkanSamplerSettings& SetMaxLod(F32 lod) { maxLod = lod; return *this; }
        inline VulkanSamplerSettings& SetMipLodBias(F32 bias) { mipLodBias = bias; return *this; }
        inline VulkanSamplerSettings& SetMipmapMode(vk::SamplerMipmapMode mode) { mipmapMode = mode; return *this; }
    };

    struct VulkanImageUploadSettings {
        const void* data = nullptr;
        Pair<U32, U32> size = { 0, 0 };
        U32 bytesPerPixel = 0;
        Pair<U32, U32> offset = { 0, 0 };
        U32 mipLevel = 0;
        U32 baseArrayLayer = 0;
        U32 layerCount = 1;
        U32 depth = 1;
        U32 depthOffset = 0;
        vk::ImageAspectFlags aspectMask = vk::ImageAspectFlagBits::eColor;
        VulkanQueueType queueType = VulkanQueueType::Graphics;

        // We can optionally provide a staging buffer for it to use,
        // if the buffer is null or smaller than required it will create 
        // its own staging buffer
        Ref<VulkanBuffer> stagingBuffer = nullptr;

        VulkanImageUploadSettings(void* data, U32 width, U32 height, U32 bytesPerPixel)
            : data(data), size(MakePair(width, height)), bytesPerPixel(bytesPerPixel) {}
        
        inline VulkanImageUploadSettings& SetOffset(U32 x, U32 y) { offset = MakePair(x, y); return *this; }
        inline VulkanImageUploadSettings& SetMipLevel(U32 level) { mipLevel = level; return *this; }
        inline VulkanImageUploadSettings& SetBaseArrayLayer(U32 layer) { baseArrayLayer = layer; return *this; }
        inline VulkanImageUploadSettings& SetLayerCount(U32 count) { layerCount = count; return *this; }
        inline VulkanImageUploadSettings& SetDepth(U32 d) { depth = d; return *this; }
        inline VulkanImageUploadSettings& SetDepthOffset(U32 d) { depthOffset = d; return *this; }
        inline VulkanImageUploadSettings& SetAspectMask(vk::ImageAspectFlags mask) { aspectMask = mask; return *this; }
        inline VulkanImageUploadSettings& SetQueueType(VulkanQueueType type) { queueType = type; return *this; }

        inline Size GetRerquireImageSize() const {
            return size.first * size.second * bytesPerPixel * layerCount * depth;
        }
    };

    struct VulkanImageAsyncUploadResult {
        Bool success = false;
        Ref<VulkanBuffer> stagingBuffer = nullptr;

        static inline VulkanImageAsyncUploadResult Faliure(const String& message = "") {
            if (!message.empty()) {
                log::Error("Failed to upload image: {}", message);
            }
            VulkanImageAsyncUploadResult result;
            result.success = false;
            return result;
        }
    };

	class VulkanImage
	{
	public:
        VulkanImage(Raw<VulkanDevice> device, const VulkanImageSettings& settings = VulkanImageSettings());
		~VulkanImage();

        Bool CreateView(const String& name, vk::ImageViewType viewType, vk::Format format, vk::ImageAspectFlags aspectMask, U32 baseMipLevel, U32 levelCount);
        Bool CreateSampler(const String& name, VulkanSamplerSettings settings = VulkanSamplerSettings());

        VulkanImageAsyncUploadResult UploadAsync(const VulkanImageUploadSettings& uploadSettings, vk::CommandBuffer commandBuffer);
        Bool UploadSync(const VulkanImageUploadSettings& uploadSettings);
        

        Bool TransitionImageLayout(
            vk::ImageLayout oldLayout,
            vk::ImageLayout newLayout,
            vk::ImageSubresourceRange subresourceRange,
            vk::PipelineStageFlags srcStageMask,
            vk::PipelineStageFlags dstStageMask,
            VulkanQueueType queueType = VulkanQueueType::Graphics
        );

        Bool TransitionImageLayoutWithCommandBuffer(
            vk::CommandBuffer commandBuffer,
            vk::ImageLayout oldLayout,
            vk::ImageLayout newLayout,
            vk::ImageSubresourceRange subresourceRange,
            vk::PipelineStageFlags srcStageMask,
            vk::PipelineStageFlags dstStageMask
        );

        inline Bool IsReady() { return m_IsReady; }
        
        inline vk::Image GetImage() { return m_Image; }
        inline vk::ImageView GetImageView(const String& name) { return m_ImageViews[name]; }
        inline vk::Sampler GetSampler(const String& name) { return m_Samplers[name]; }
        

	private:
        Bool Recreate();
        void Cleanup();

        vk::ImageCreateInfo GetCreateInfo();

	private:
		Raw<VulkanDevice> m_Device = nullptr;
        VulkanImageSettings m_Settings;

        Ref<VulkanBuffer> m_StagingBuffer = nullptr;

        vk::ImageLayout m_CurrentLayout = vk::ImageLayout::eUndefined;
        Bool m_IsReady = false;

        vk::Image m_Image = VK_NULL_HANDLE;
        vk::DeviceMemory m_Memory = VK_NULL_HANDLE;
        Map<String, vk::ImageView> m_ImageViews;
        Map<String, vk::Sampler> m_Samplers;
	};


}