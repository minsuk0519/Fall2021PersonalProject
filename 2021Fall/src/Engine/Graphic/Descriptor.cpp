#include "Descriptor.hpp"
#include "Engine/Misc/helper.hpp"

//standard library
#include <stdexcept>

void DescriptorManager::init()
{
	CreatevulkanShaderModule("data/shaders/baserendervert.spv");
	CreatevulkanShaderModule("data/shaders/baserenderfrag.spv");
	CreatevulkanShaderModule("data/shaders/deferredvert.spv");
	CreatevulkanShaderModule("data/shaders/deferredfrag.spv");

	SetupDescriptorSetLayouts();
	SetupPipelineLayouts();
	SetupDescriptorPool();
}

void DescriptorManager::close()
{
	for (auto setlayout : vulkanDescriptorSetLayouts)
	{
		vkDestroyDescriptorSetLayout(vulkanDevice, setlayout, nullptr);
	}
	vulkanDescriptorSetLayouts.clear();

	for (auto pipelinelayout : vulkanPipelineLayouts)
	{
		vkDestroyPipelineLayout(vulkanDevice, pipelinelayout, nullptr);
	}
	vulkanPipelineLayouts.clear();

	vkDestroyDescriptorPool(vulkanDevice, vulkanDescriptorPool, nullptr);
}

void DescriptorManager::SetupShaderModules()
{
	
}

void DescriptorManager::SetupDescriptorSetLayouts()
{
	//TODO : implement here when adding shadermodule
	//VkDescriptorSetLayoutCreateInfo layoutInfo{};
	//layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	//layoutInfo.bindingCount = static_cast<uint32_t>(layoutbindings.size());
	//layoutInfo.pBindings = layoutbindings.data();

	//if (vkCreateDescriptorSetLayout(vulkanDevice, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
	//{
	//	throw std::runtime_error("failed to create descriptor set layout!");
	//}
}

void DescriptorManager::SetupPipelineLayouts()
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(vulkanDescriptorSetLayouts.size());
	pipelineLayoutInfo.pSetLayouts = vulkanDescriptorSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	if (vkCreatePipelineLayout(vulkanDevice, &pipelineLayoutInfo, nullptr, vulkanPipelineLayouts.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create pipelinelayout");
	}
}

void DescriptorManager::SetupDescriptorPool()
{
}

void DescriptorManager::CreatevulkanShaderModule(const char* filename)
{
	VkShaderModule shaderModule;

	std::vector<char> code = Helper::readFile(filename);

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	if (vkCreateShaderModule(vulkanDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}

	vulkanShaderModules.push_back(shaderModule);
}
