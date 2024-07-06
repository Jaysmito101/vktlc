#include "vulkanapi/VulkanDevice.hpp"
#include "vulkanapi/VulkanShader.hpp"

namespace tlc
{

	VulkanShaderModule::VulkanShaderModule(Raw<VulkanDevice> device, const List<U32>& shaderCode)
	{
		m_Device = device;

		vk::ShaderModuleCreateInfo createInfo = vk::ShaderModuleCreateInfo()
			.setCodeSize(static_cast<U32>(shaderCode.size()) * 4)
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
			m_Device->WaitIdle();
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