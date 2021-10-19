#pragma once

//3rd party library
#include <vulkan/vulkan.h>

//standard library
#include <vector>

class GraphicPipeline
{
public:
	GraphicPipeline(VkDevice device);

	void init(VkRenderPass renderpass, VkDescriptorSetLayout descriptorSetLayout);
	void close();

	VkPipeline GetPipeline() const;
	VkPipelineLayout GetPipelinLayout() const;
private:
	VkDevice vulkanDevice;

	VkPipelineLayout vulkanpipelineLayout;
	VkPipeline vulkanPipeline;

	VkShaderModule CreatevulkanShaderModule(const std::vector<char>& code);
};