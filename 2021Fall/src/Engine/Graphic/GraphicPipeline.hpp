#pragma once

//3rd party library
#include <vulkan/vulkan.h>

//standard library
#include <vector>

class GraphicPipeline
{
public:
	GraphicPipeline(VkDevice device);

	void init(VkRenderPass renderpass, VkPipelineLayout pipelinelayout, VkSampleCountFlagBits msaaSamples, VkPipelineVertexInputStateCreateInfo inputstate, uint32_t colorNum, std::vector<VkPipelineShaderStageCreateInfo> shadermodules,
		uint32_t width, uint32_t height);
	void close();

	VkPipeline GetPipeline() const;
private:
	VkDevice vulkanDevice;

	VkPipeline vulkanPipeline;

	VkPipelineCache vulkanpipelinecache = VK_NULL_HANDLE;

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
};