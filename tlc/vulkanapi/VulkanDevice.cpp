#include "vulkanapi/VulkanDevice.hpp"
#include "vulkanapi/VulkanContext.hpp"
#include "vulkanapi/VulkanShader.hpp"
#include "vulkanapi/VulkanSwapchain.hpp"
#include "vulkanapi/VulkanBuffer.hpp"

namespace tlc
{
	VulkanDevice::VulkanDevice(VulkanContext* parentContext, vk::PhysicalDevice physicalDevice, const VulkanDeviceSettings& settings, vk::SurfaceKHR surface)
		: m_ParentContext(parentContext)
		, m_PhysicalDevice(physicalDevice)
		, m_Settings(settings)
	{
		log::Debug("Creating Vulkan Logical Device");
		if (!CreateDevice(surface))
		{
			log::Error("Failed to create device");
			return;
		}

		log::Info("Vulkan Logical Device created");

		m_IsReady = true;
	}

	VulkanDevice::~VulkanDevice()
	{
		if(m_IsReady) {
			Cleanup();
		}
	}

	void VulkanDevice::Cleanup() {
		if (!m_IsReady) {
			log::Trace("Device is not ready! Cannot cleanup.");
			return;
		}

		(void)m_Device.waitIdle();


		for (auto& commandPool : m_CommandPools)
		{
			if (commandPool == static_cast<vk::CommandPool>(VK_NULL_HANDLE)) continue;
			m_Device.destroyCommandPool(commandPool);
		}

		for(const auto& [_, pools] : m_AvailableDescriptorPools) {
			for (const auto& pool: pools) {
				m_Device.destroyDescriptorPool(pool);
			}
		}

		for(const auto& [_, poolGroups] : m_DescriptorPools) {
			for(const auto& [_, pools] : poolGroups) {
				for (const auto& pool: pools) {
					m_Device.destroyDescriptorPool(pool);
				}
			}
		}

		for (const auto& [_, layout] : m_DescriptorSetLayoutCache) {
			m_Device.destroyDescriptorSetLayout(layout);
		}
		m_DescriptorSetLayoutCache.clear();

		m_Device.destroy();
	}

	Ref<VulkanShaderModule> VulkanDevice::CreateShaderModule(const List<U32>& shaderCode)
	{
		if (!m_IsReady)
		{
			log::Error("Device is not ready");
			return nullptr;
		}
		return CreateRef<VulkanShaderModule>(this, shaderCode);
	}

	vk::Semaphore VulkanDevice::CreateVkSemaphore(vk::SemaphoreCreateFlags flags) const
	{
		auto semaphoreCreateInfo = vk::SemaphoreCreateInfo()
			.setFlags(flags);

		auto [result, semaphore] = m_Device.createSemaphore(semaphoreCreateInfo);
		VkCall(result);
		return semaphore;
	}

	void VulkanDevice::DestroyVkSemaphore(vk::Semaphore semaphore) const
	{
		WaitIdle();
		m_Device.destroySemaphore(semaphore);
	}

	vk::Fence VulkanDevice::CreateVkFence(vk::FenceCreateFlags flags) const
	{
		auto fenceCreateInfo = vk::FenceCreateInfo()
			.setFlags(flags);

		auto [result, fence] = m_Device.createFence(fenceCreateInfo);
		VkCall(result);
		return fence;
	}

	void VulkanDevice::DestroyVkFence(vk::Fence fence) const
	{
		WaitIdle();
		m_Device.destroyFence(fence);
	}

	U32 VulkanDevice::FindMemoryType(U32 typeFilter, vk::MemoryPropertyFlags properties) const
	{
		auto memoryProperties = m_PhysicalDevice.getMemoryProperties();

		for (U32 i = 0; i < memoryProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		log::Error("Failed to find suitable memory type");
		return std::numeric_limits<U32>::max();
	}

	Bool VulkanDevice::CreateDevice(vk::SurfaceKHR surface)
	{

		vk::DeviceCreateInfo deviceCreateInfo = vk::DeviceCreateInfo();

		List<vk::DeviceQueueCreateInfo> queueCreateInfos;

		if ((m_QueueFamilyIndices[Present] = m_QueueFamilyIndices[Graphics] = FindAndAddQueueCreateInfo(m_Settings.requireGraphicsQueue, vk::QueueFlagBits::eGraphics, &m_Settings.graphicsQueuePriority, queueCreateInfos, surface)) == -1)
		{
			log::Warn("Failed to find graphics queue family");
		}

		if ((m_QueueFamilyIndices[Compute] = FindAndAddQueueCreateInfo(m_Settings.requireComputeQueue, vk::QueueFlagBits::eCompute, &m_Settings.computeQueuePriority, queueCreateInfos)) == -1)
		{
			log::Warn("Failed to find compute queue family");
		}

		if ((m_QueueFamilyIndices[Transfer] = FindAndAddQueueCreateInfo(m_Settings.requireTransferQueue, vk::QueueFlagBits::eTransfer, &m_Settings.transferQueuePriority, queueCreateInfos)) == -1)
		{
			log::Warn("Failed to find transfer queue family");
		}

		deviceCreateInfo.setQueueCreateInfoCount(static_cast<U32>(queueCreateInfos.size()))
			.setPQueueCreateInfos(queueCreateInfos.data())
			.setEnabledLayerCount(0);

		// NOTE: I am not checking for extensions support here as the target devices for this engine support must swapchain extension
		List<CString> extensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		deviceCreateInfo.setEnabledExtensionCount(static_cast<U32>(extensions.size()))
			.setPpEnabledExtensionNames(extensions.data());

		if (m_Settings.enableValidationLayers)
		{
			const auto& layers = m_ParentContext->GetLayers();
			deviceCreateInfo.setEnabledLayerCount(static_cast<U32>(layers.size()))
				.setPpEnabledLayerNames(layers.data());
		}

		auto [result, device] = m_PhysicalDevice.createDevice(deviceCreateInfo);
		VkCritCall(result);
		m_Device = device;

		if (m_Device == static_cast<vk::Device>(VK_NULL_HANDLE))
		{
			log::Error("Failed to create logical device");
			return false;
		}

		for (U32 i = 0; i < VulkanQueueType::Count; i++)
		{
			if (m_QueueFamilyIndices[i] == -1 || m_QueueFamilyIndices[i] == 1000) continue;
			m_Queues[i] = m_Device.getQueue(m_QueueFamilyIndices[i], 0);
			if (m_Queues[i] == static_cast<vk::Queue>(VK_NULL_HANDLE))
			{
				log::Error("Failed to get queue");
				return false;
			}
		}

		if (!CreateCommandPools())
		{
			log::Error("Failed to create command pools");
			return false;
		}

		return true;
	}

	Bool VulkanDevice::CreateCommandPools()
	{
		auto poolCreateInfo = vk::CommandPoolCreateInfo()
			.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);


		for (U32 i = 0; i < VulkanQueueType::Count; i++)
		{
			if (m_QueueFamilyIndices[i] == -1 || m_QueueFamilyIndices[i] == 1000)
			{
				m_CommandPools[i] = VK_NULL_HANDLE;
				continue;
			}
			poolCreateInfo.setQueueFamilyIndex(m_QueueFamilyIndices[i]);
			
			auto [result, commandPool] = m_Device.createCommandPool(poolCreateInfo);
			VkCritCall(result);
			m_CommandPools[i] = commandPool;

			if (m_CommandPools[i] == static_cast<vk::CommandPool>(VK_NULL_HANDLE))
			{
				log::Error("Failed to create command pool");
				return false;
			}
		}

		return true;
	}

	I32 VulkanDevice::FindAndAddQueueCreateInfo(Bool enable, const vk::QueueFlags& flags, F32* queuePriority, List<vk::DeviceQueueCreateInfo>& queueCreateInfos, vk::SurfaceKHR surface)
	{
		if (!enable) return 1000;

		I32 queueFamilyIndex = FindQueueFamily(m_PhysicalDevice, flags, flags & vk::QueueFlagBits::eGraphics ? surface : nullptr);
		if (queueFamilyIndex == -1)
		{
			log::Warn("Failed to find queue family");
			return queueFamilyIndex;
		}

		if (m_UniqeQueueFamiliesIndices.find(queueFamilyIndex) != m_UniqeQueueFamiliesIndices.end())
		{
			log::Warn("Queue family already exists");
			return queueFamilyIndex;
		};
		m_UniqeQueueFamiliesIndices.insert(queueFamilyIndex);

		queueCreateInfos.push_back(CreateQueueCreateInfo(queueFamilyIndex, queuePriority));
		return queueFamilyIndex;
	}

	I32 VulkanDevice::FindQueueFamily(const vk::PhysicalDevice& physicalDevice, const vk::QueueFlags& flags, const vk::SurfaceKHR& surface)
	{
		auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

		for (I32 i = 0; i < queueFamilyProperties.size(); ++i)
		{
			if (queueFamilyProperties[i].queueFlags & flags)
			{
				if (surface)
				{
					if (physicalDevice.getSurfaceSupportKHR(i, surface).value)
					{
						return i;
					}
				}
				else
				{
					return i;
				}
			}
		}

		return -1;
	}

	vk::DeviceQueueCreateInfo VulkanDevice::CreateQueueCreateInfo(I32 queueFamilyIndex, F32* queuePriority)
	{
		vk::DeviceQueueCreateInfo queueCreateInfo = vk::DeviceQueueCreateInfo()
			.setQueueFamilyIndex(queueFamilyIndex)
			.setQueueCount(1)
			.setPQueuePriorities(queuePriority);
		return queueCreateInfo;
	}

	vk::DescriptorPool VulkanDevice::CreateDescriptorPool(vk::DescriptorType type) {
		auto availablePools = m_AvailableDescriptorPools.find(type);
		if (availablePools != m_AvailableDescriptorPools.end()) {
			auto pool = availablePools->second.back();
			availablePools->second.pop_back();
			return pool;
		}

		static const U32 descriptorPoolSize = 1000;

		auto poolSizes = vk::DescriptorPoolSize()
			.setType(type)
			.setDescriptorCount(descriptorPoolSize);

		auto descriptorPoolCreateInfo = vk::DescriptorPoolCreateInfo()
			.setMaxSets(1000)
			.setPoolSizeCount(1)
			.setPPoolSizes(&poolSizes);
			
		auto [result, pool] = m_Device.createDescriptorPool(descriptorPoolCreateInfo);
		VkCritCall(result);

		return pool;
	}

	List<vk::DescriptorPool>& VulkanDevice::GrabDescriptorPools(const String& group, vk::DescriptorType type) {
		auto typeGroup = m_DescriptorPools.find(group);
		if (typeGroup == m_DescriptorPools.end()) {
			m_DescriptorPools.insert_or_assign(group, UnorderedMap<vk::DescriptorType, List<vk::DescriptorPool>>());
			typeGroup =  m_DescriptorPools.find(group);
		}

		auto poolArray = typeGroup->second.find(type);
		if (poolArray == typeGroup->second.end()) {
			typeGroup->second.insert_or_assign(type, List<vk::DescriptorPool>{ CreateDescriptorPool(type)});
			poolArray =  typeGroup->second.find(type);
		}

		return poolArray->second;
	}

	Bool VulkanDevice::ExpandDescriptorPool(const String& group, vk::DescriptorType type) {
		auto& pools = GrabDescriptorPools(group, type);
		pools.push_back(CreateDescriptorPool(type));
		return true;
	}

	List<vk::DescriptorSet> VulkanDevice::AllocateDescriptorSets(const String& group, vk::DescriptorType type, const List<vk::DescriptorSetLayout>& descriptorSetLayouts) {
		const auto& pools = GrabDescriptorPools(group, type);

		auto descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(pools.back())
			.setDescriptorSetCount(static_cast<U32>(descriptorSetLayouts.size()))
			.setSetLayouts(descriptorSetLayouts);
		auto [result, descriptorSet] = m_Device.allocateDescriptorSets(descriptorSetAllocateInfo);

		switch (result) {
			case vk::Result::eSuccess: 
				return descriptorSet;
			case vk::Result::eErrorFragmentedPool:
			case vk::Result::eErrorOutOfPoolMemory:
				if(!ExpandDescriptorPool(group, type)) {
					return {};
				}
				return AllocateDescriptorSets(group, type, descriptorSetLayouts);
			default:
				VkCall(result);
				return {};
		}
	}

	void VulkanDevice::FreeDescriptorGroup(const String& group) {
		// reset the descriptor pools from the group and push them, in the avialable pools map

		if (!m_DescriptorPools.contains(group)) {
			log::Trace("Descriptor group: {} doesnt exist!", group);
			return;
		}

		auto allPools = m_DescriptorPools.at(group);
		m_DescriptorPools.erase(group);

		for(const auto& [poolType, pools] : allPools) {
			auto& availablePools = m_AvailableDescriptorPools[poolType]; // get it or a default empty list
			for (const auto& pool: pools) {
				m_Device.resetDescriptorPool(pool);
				availablePools.push_back(pool);
			}
		}
		allPools.clear();
	}

	// NOTE: assumption bindings are sorted
	Size VulkanDevice::CalculateDescriptorLayoutCreateInfoHash(const vk::DescriptorSetLayoutCreateInfo& createInfo) {
		auto result = std::hash<Size>()(createInfo.bindingCount);
		for (Size i = 0; i < createInfo.bindingCount; i ++) {
			auto b = createInfo.pBindings[i];
			Size bindingHash = (Size)b.binding | (Size)b.descriptorType << 8 | (Size)b.descriptorCount << 16 | (U32)b.stageFlags << 24;
			result ^= std::hash<Size>()(bindingHash);
		}
		return result;
	}

	vk::DescriptorSetLayout VulkanDevice::CreateDescriptorSetLayout(const vk::DescriptorSetLayoutCreateInfo& createInfo) {
		auto hash = CalculateDescriptorLayoutCreateInfoHash(createInfo);
		auto cacheEntry = m_DescriptorSetLayoutCache.find(hash);
		if (cacheEntry != m_DescriptorSetLayoutCache.end()) {
			return cacheEntry->second;
		}

		auto [result, layout] = m_Device.createDescriptorSetLayout(createInfo);
		VkCall(result);

		m_DescriptorSetLayoutCache.insert_or_assign(hash, layout);

		return layout;
	}
	

}