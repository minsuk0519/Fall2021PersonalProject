#pragma once

//3rd party library
#include <vulkan/vulkan.h>

//standard library
#include <vector>

class GraphicPipeline
{
public:
	GraphicPipeline(VkDevice device);

	void init(VkRenderPass renderpass, VkPipelineLayout pipelinelayout, VkSampleCountFlagBits msaaSamples, VkPipelineVertexInputStateCreateInfo inputstate, uint32_t colorNum, std::vector<VkPipelineShaderStageCreateInfo> shadermodules);
	void close();

	VkPipeline GetPipeline() const;

	void AddShaderStages(const char* shaderpath, VkShaderStageFlagBits flag);
private:
	VkDevice vulkanDevice;

	VkPipeline vulkanPipeline;

	VkPipelineCache vulkanpipelinecache = VK_NULL_HANDLE;

	VkShaderModule CreatevulkanShaderModule(const std::vector<char>& code);

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
};