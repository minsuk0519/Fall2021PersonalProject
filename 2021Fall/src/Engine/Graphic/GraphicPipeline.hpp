#pragma once

//3rd party library
#include <vulkan/vulkan.h>

//standard library
#include <vector>

class GraphicPipeline
{
public:
	GraphicPipeline(VkDevice device);

	void init(VkRenderPass renderpass);
	void close();

	VkPipeline GetPipeline() const;
private:
	VkDevice vulkanDevice;

	VkPipelineLayout vulkanpipelineLayout;
	VkPipeline vulkanPipeline;

	VkShaderModule CreatevulkanShaderModule(const std::vector<char>& code);
};