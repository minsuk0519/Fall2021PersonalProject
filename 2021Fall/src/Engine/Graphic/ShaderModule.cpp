#include "ShaderModule.hpp"

//standard library
#include <stdexcept>

ShaderModule::ShaderModule(VkDevice device) : vulkanDevice(device) {}

void ShaderModule::init(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	if(vkCreateShaderModule(vulkanDevice, &createInfo, nullptr, &vulkanShaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}
}

void ShaderModule::close()
{
	vkDestroyShaderModule(vulkanDevice, vulkanShaderModule, nullptr);
}

VkShaderModule ShaderModule::getvulkanModule()
{
	return vulkanShaderModule;
}
