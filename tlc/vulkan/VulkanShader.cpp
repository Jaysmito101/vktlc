#include "vulkan/VulkanDevice.hpp"
#include "vulkan/VulkanShader.hpp"

namespace tlc
{

	VulkanShaderModule::VulkanShaderModule(VulkanDevice* device, const List<U8>& shaderCode)
	{
		m_Device = device;

		vk::ShaderModuleCreateInfo createInfo = vk::ShaderModuleCreateInfo()
			.setCodeSize(static_cast<U32>(shaderCode.size()))
			.setPCode(reinterpret_cast<const U32*>(shaderCode.data()));

		if (m_Device->GetDevice().createShaderModule(&createInfo, nullptr, &m_ShaderModule) != vk::Result::eSuccess)
		{
			log::Error("Failed to create shader module");
			return;
		}

		m_IsReady = true;

	}

	VulkanShaderModule::~VulkanShaderModule()
	{
		if (m_IsReady)
		{
			m_Device->GetDevice().destroyShaderModule(m_ShaderModule);
		}
	}

	vk::PipelineShaderStageCreateInfo VulkanShaderModule::GetShaderStageCreateInfo(vk::ShaderStageFlagBits stage) const
	{
		return vk::PipelineShaderStageCreateInfo()
			.setStage(stage)
			.setModule(m_ShaderModule)
			.setPName("main");
	}

}