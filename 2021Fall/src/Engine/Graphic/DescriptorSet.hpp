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
	void BindDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);

	VkDescriptorSetLayout GetSetLayout() const;

private:
	VkDevice vulkanDevice;
	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSet;

	std::vector<Descriptor> descriptors;
};