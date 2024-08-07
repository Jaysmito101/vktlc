#pragma once

#include "vulkanapi/VulkanBase.hpp"

namespace tlc
{

	class VulkanDevice;

	class VulkanShaderModule
	{
	public:
		VulkanShaderModule(Raw<VulkanDevice> device, const List<U32>& shaderCode);
		~VulkanShaderModule();

		vk::PipelineShaderStageCreateInfo GetShaderStageCreateInfo(vk::ShaderStageFlagBits stage) const;

		inline const vk::ShaderModule GetShaderModule() const { return m_ShaderModule; }
		inline Bool IsReady() const { return m_IsReady; }

	private:
		Raw<VulkanDevice> m_Device;
		vk::ShaderModule m_ShaderModule = VK_NULL_HANDLE;
		Bool m_IsReady = false;
	};

}