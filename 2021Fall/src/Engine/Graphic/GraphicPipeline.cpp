#include "GraphicPipeline.hpp"
#include "Engine/helper.hpp"

GraphicPipeline::GraphicPipeline(VkDevice device) : vulkanDevice(device) {}

void GraphicPipeline::init()
{
	vertShaderModule = new ShaderModule(vulkanDevice);
	vertShaderModule->init(Helper::readFile("data/shaders/simpletrianglevert.spv"));
	fragShaderModule = new ShaderModule(vulkanDevice);
	fragShaderModule->init(Helper::readFile("data/shaders/simpletrianglefrag.spv"));

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule->getvulkanModule();
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule->getvulkanModule();
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	vertShaderModule->close();
	delete vertShaderModule;

	fragShaderModule->close();
	delete fragShaderModule;
}

void GraphicPipeline::close()
{
}
