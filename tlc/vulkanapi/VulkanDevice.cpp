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
		CheckExtensionSupport(physicalDevice);
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
		if (m_IsReady) {
			Cleanup();
		}
	}

	void VulkanDevice::Cleanup() {
		if (!m_IsReady) {
			log::Trace("Device is not ready! Cannot cleanup.");
			return;
		}

		(void)m_Device.waitIdle();

		for (auto queryPool : m_QueryPools) {
			if (queryPool == static_cast<vk::QueryPool>(VK_NULL_HANDLE)) continue;
			m_Device.destroyQueryPool(queryPool);
		}

		for (auto& commandPool : m_CommandPools)
		{
			if (commandPool == static_cast<vk::CommandPool>(VK_NULL_HANDLE)) continue;
			m_Device.destroyCommandPool(commandPool);
		}

		for (const auto& [_, pools] : m_AvailableDescriptorPools) {
			for (const auto& pool : pools) {
				m_Device.destroyDescriptorPool(pool);
			}
		}

		for (const auto& [_, poolGroups] : m_DescriptorPools) {
			for (const auto& [_, pools] : poolGroups) {
				for (const auto& pool : pools) {
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

	vk::QueryPool VulkanDevice::CreateQueryPool(vk::QueryType type, U32 count) {
		auto pipelineStatisticsFlags = vk::QueryPipelineStatisticFlagBits::eClippingInvocations
			| vk::QueryPipelineStatisticFlagBits::eComputeShaderInvocations
			| vk::QueryPipelineStatisticFlagBits::eMeshShaderInvocationsEXT
			| vk::QueryPipelineStatisticFlagBits::eTaskShaderInvocationsEXT
			| vk::QueryPipelineStatisticFlagBits::eVertexShaderInvocations;

		auto queryPoolCreateInfo = vk::QueryPoolCreateInfo()
			.setQueryType(type)
			.setQueryCount(count)
			.setPipelineStatistics(type == vk::QueryType::ePipelineStatistics ? pipelineStatisticsFlags : vk::QueryPipelineStatisticFlagBits(0));

		auto [result, queryPool] = m_Device.createQueryPool(queryPoolCreateInfo);
		VkCall(result);
			
		m_QueryPools.push_back(queryPool);

		return queryPool;
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

	void VulkanDevice::CheckExtensionSupport(vk::PhysicalDevice physicalDevice) {
		U32 extensionCount = 0;
		VkCall(physicalDevice.enumerateDeviceExtensionProperties(nullptr, &extensionCount, nullptr));

		List<vk::ExtensionProperties> availableExtensions(extensionCount);
		VkCall(physicalDevice.enumerateDeviceExtensionProperties(nullptr, &extensionCount, availableExtensions.data()));

		log::Trace("Available Vulkan device extensions:");
		for (const auto& extension : availableExtensions)
		{
			log::Trace("{}", extension.extensionName.data());

			if (strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
			{
				log::Info("Swapchain extension is supported");
				m_SwapchanSupported = true;
			}
			else if (strcmp(extension.extensionName, VK_EXT_MESH_SHADER_EXTENSION_NAME) == 0)
			{
				log::Info("Ray tracing extension is supported");
				m_MeshShadingSupported = true;
			}
			else if (strcmp(extension.extensionName, VK_KHR_RAY_QUERY_EXTENSION_NAME) == 0)
			{
				log::Info("Ray query extension is supported");
				m_RayTracingSupported = true;
			}

		}
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

		if (!m_SwapchanSupported)
		{
			log::Fatal("Swapchain extension is not supported");
		}

		if (!m_MeshShadingSupported) {
			log::Fatal("Mesh shading extension is not supported");
		}


		deviceCreateInfo.setQueueCreateInfoCount(static_cast<U32>(queueCreateInfos.size()))
			.setPQueueCreateInfos(queueCreateInfos.data())
			.setEnabledLayerCount(0);

		List<CString> extensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		};

		if (m_MeshShadingSupported)
		{
			extensions.push_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);
		}

		if (m_RayTracingSupported)
		{
			extensions.push_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);
			extensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
			extensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
		}

		deviceCreateInfo.setEnabledExtensionCount(static_cast<U32>(extensions.size()))
			.setPpEnabledExtensionNames(extensions.data());

		if (m_Settings.enableValidationLayers)
		{
			const auto& layers = m_ParentContext->GetLayers();
			deviceCreateInfo.setEnabledLayerCount(static_cast<U32>(layers.size()))
				.setPpEnabledLayerNames(layers.data());
		}

		auto featuresAccelerationStructure = vk::PhysicalDeviceAccelerationStructureFeaturesKHR()
			.setAccelerationStructure(true)
			.setPNext(nullptr);

		auto featuresRayQueries = vk::PhysicalDeviceRayQueryFeaturesKHR()
			.setRayQuery(true)
			.setPNext(&featuresAccelerationStructure);

		auto featureMeshShading = vk::PhysicalDeviceMeshShaderFeaturesEXT()
			.setTaskShader(true)
			.setMeshShader(true)
			.setPNext(m_RayTracingSupported ? &featuresRayQueries : nullptr);

		auto features14 = vk::PhysicalDeviceVulkan14Features()
			.setMaintenance5(true)
			.setMaintenance6(true)
			.setPNext(m_MeshShadingSupported ? (void*)&featureMeshShading : m_RayTracingSupported ? (void*)&featuresRayQueries : nullptr);

		auto features13 = vk::PhysicalDeviceVulkan13Features()
			.setSynchronization2(true)
			.setDynamicRendering(true)
			//.setMaintenance4(true)
			.setPNext(&features14);

		auto features12 = vk::PhysicalDeviceVulkan12Features()
			.setDrawIndirectCount(true)
			.setStorageBuffer8BitAccess(true)
			.setUniformAndStorageBuffer8BitAccess(true)
			.setShaderFloat16(true)
			.setShaderInt8(true)
			.setSamplerFilterMinmax(true)
			.setScalarBlockLayout(true)
			.setBufferDeviceAddress(m_RayTracingSupported)
			.setDescriptorIndexing(true)
			.setShaderSampledImageArrayNonUniformIndexing(true)
			.setDescriptorBindingSampledImageUpdateAfterBind(true)
			.setDescriptorBindingUpdateUnusedWhilePending(true)
			.setDescriptorBindingPartiallyBound(true)
			.setDescriptorBindingVariableDescriptorCount(true)
			.setRuntimeDescriptorArray(true)
			.setPNext(&features13);

		auto features11 = vk::PhysicalDeviceVulkan11Features()
			.setStorageBuffer16BitAccess(true)
			.setShaderDrawParameters(true)
			.setPNext(&features12);

		auto features = vk::PhysicalDeviceFeatures()
			.setMultiDrawIndirect(true)
			.setPipelineStatisticsQuery(true)
			.setSamplerAnisotropy(true)
			.setShaderFloat64(true)
			.setShaderInt64(true)
			.setShaderInt16(true);

		auto features2 = vk::PhysicalDeviceFeatures2()
			.setFeatures(features)
			.setPNext(&features11);

		deviceCreateInfo.setPNext(&features2);

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

		static const U32 descriptorPoolSize = 100;

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
			typeGroup = m_DescriptorPools.find(group);
		}

		auto poolArray = typeGroup->second.find(type);
		if (poolArray == typeGroup->second.end()) {
			typeGroup->second.insert_or_assign(type, List<vk::DescriptorPool>{ CreateDescriptorPool(type)});
			poolArray = typeGroup->second.find(type);
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
			if (!ExpandDescriptorPool(group, type)) {
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

		for (const auto& [poolType, pools] : allPools) {
			auto& availablePools = m_AvailableDescriptorPools[poolType]; // get it or a default empty list
			for (const auto& pool : pools) {
				m_Device.resetDescriptorPool(pool);
				availablePools.push_back(pool);
			}
		}
		allPools.clear();
	}

	// NOTE: assumption bindings are sorted
	Size VulkanDevice::CalculateDescriptorLayoutCreateInfoHash(const vk::DescriptorSetLayoutCreateInfo& createInfo) {
		auto result = std::hash<Size>()(createInfo.bindingCount);
		for (Size i = 0; i < createInfo.bindingCount; i++) {
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
