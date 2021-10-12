#pragma once
//3rd party library
#include <vulkan/vulkan.h>

//vector
#include <vector>

class ShaderModule
{
public:
	ShaderModule(VkDevice device);
	void init(const std::vector<char>& code);
	void close();

	VkShaderModule getvulkanModule();
private:
	VkShaderModule vulkanShaderModule;
	VkDevice vulkanDevice;
};