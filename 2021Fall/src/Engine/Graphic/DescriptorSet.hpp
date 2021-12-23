#pragma once

//3rd party library
#include <vulkan/vulkan.h>

//standard library
#include <vector>

struct Descriptor;

class DescriptorManager;

class DescriptorSet
{
public:
	DescriptorSet(VkDevice device);

	void init();
	void close();

public:
	void BindDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, std::vector<uint32_t> offset);
	void BindDescriptorSetNoIndex(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);

private:
	friend class DescriptorManager;

	VkDevice vulkanDevice;
	VkDescriptorSet descriptorSet;

	uint32_t dynamic_count = 0;
	std::vector<uint32_t> dynamic_offset;

	uint32_t currentindex = 0;
};