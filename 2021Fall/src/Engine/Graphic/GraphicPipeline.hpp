#pragma once

//3rd party library
#include <vulkan/vulkan.h>

//standard library
#include <vector>

class GraphicPipeline
{
public:
	GraphicPipeline(VkDevice device);

	void init(VkRenderPass renderpass, VkDescriptorSetLayout descriptorSetLayout, VkSampleCountFlagBits msaaSamples, VkPipelineVertexInputStateCreateInfo inputstate, uint32_t colorNum);
	void close();

	VkPipeline GetPipeline() const;
	VkPipelineLayout GetPipelinLayout() const;

	void AddShaderStages(const char* shaderpath, VkShaderStageFlagBits flag);
private:
	VkDevice vulkanDevice;

	VkPipelineLayout vulkanpipelineLayout;
	VkPipeline vulkanPipeline;

	VkShaderModule CreatevulkanShaderModule(const std::vector<char>& code);

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
};