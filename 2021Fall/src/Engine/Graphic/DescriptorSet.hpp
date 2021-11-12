#pragma once

//3rd party library
#include <vulkan/vulkan.h>

//standard library
#include <vector>

class DescriptorSet
{
public:
	DescriptorSet(VkDevice device);

	void init();
	void close();

public:
	struct Descriptor
	{
		VkDescriptorType type;
		uint32_t binding;
		VkShaderStageFlags stage;

		VkDescriptorBufferInfo bufferInfo;
		VkDescriptorImageInfo imageInfo;
	};

	void AddDescriptor(Descriptor des);
	void CreateDescriptorSet();
	void BindDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t offset);

	VkDescriptorSetLayout GetSetLayout() const;

private:
	VkDevice vulkanDevice;
	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSet;

	uint32_t dynamic_count = 0;
	std::vector<uint32_t> dynamic_offset;

	std::vector<Descriptor> descriptors;
};