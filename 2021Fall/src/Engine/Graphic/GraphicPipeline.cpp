#include "GraphicPipeline.hpp"
#include "Engine/helper.hpp"

GraphicPipeline::GraphicPipeline(VkDevice device) : vulkanDevice(device) {}

void GraphicPipeline::init()
{
	VkShaderModule vertShaderModule = CreatevulkanShaderModule(Helper::readFile("data/shaders/simpletrianglevert.spv"));
	VkShaderModule fragShaderModule= CreatevulkanShaderModule(Helper::readFile("data/shaders/simpletrianglefrag.spv"));

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };


	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;


	vkDestroyShaderModule(vulkanDevice, vertShaderModule, nullptr);
	vkDestroyShaderModule(vulkanDevice, fragShaderModule, nullptr);
}

void GraphicPipeline::close()
{
}

VkShaderModule GraphicPipeline::CreatevulkanShaderModule(const std::vector<char>& code)
{
	VkShaderModule shaderModule;

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	if (vkCreateShaderModule(vulkanDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}
