#pragma once

//3rd party library
#include <vulkan/vulkan.h>

//standard library
#include <vector>

class DescriptorManager
{
public:
	void init();
	void close();

private:
	std::vector<VkDescriptorSetLayout> vulkanDescriptorSetLayouts;
	std::vector<VkPipelineLayout> vulkanPipelineLayouts;

	std::vector<VkShaderModule> vulkanShaderModules;

	VkDescriptorPool vulkanDescriptorPool;

	VkDevice vulkanDevice;
	
	void SetupShaderModules();
	void SetupDescriptorSetLayouts();
	void SetupPipelineLayouts();
	void SetupDescriptorPool();

	void CreatevulkanShaderModule(const char* filename);
};

struct Program
{
	VkShaderModule shadermodule;

	VkDescriptorType type;
	uint32_t descriptcount;
};

struct Descriptor
{
	VkDescriptorType type;
	uint32_t binding;
	VkShaderStageFlags stage;

	VkDescriptorBufferInfo bufferInfo;
	VkDescriptorImageInfo imageInfo;
};