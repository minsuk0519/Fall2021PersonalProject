#pragma once

//3rd party library
#include <vulkan/vulkan.h>

//standard library
#include <vector>

class GraphicPipeline
{
public:
	GraphicPipeline(VkDevice device);

	void init();
	void close();
private:
	VkDevice vulkanDevice;

	VkPipelineLayout pipelineLayout;

	VkShaderModule CreatevulkanShaderModule(const std::vector<char>& code);
};