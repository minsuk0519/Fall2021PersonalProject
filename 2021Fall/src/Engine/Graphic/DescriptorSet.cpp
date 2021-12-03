#include "DescriptorSet.hpp"
#include "Descriptor.hpp"

//standard library
#include <stdexcept>
#include <array>

DescriptorSet::DescriptorSet(VkDevice device) : vulkanDevice(device) {}

void DescriptorSet::close()
{
    vkDestroyDescriptorSetLayout(vulkanDevice, descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(vulkanDevice, descriptorPool, nullptr);
}

void DescriptorSet::AddDescriptor(Descriptor des)
{
	descriptors.push_back(des);
}

void DescriptorSet::CreateDescriptorSet()
{
    std::vector<VkDescriptorPoolSize> poolSizes{};
    std::vector<VkDescriptorSetLayoutBinding> layoutbindings;

    for (const auto& descriptor : descriptors)
    {
        VkDescriptorPoolSize poolsize;
        poolsize.type = descriptor.type;
        poolsize.descriptorCount = 1;
        poolSizes.push_back(poolsize);

        VkDescriptorSetLayoutBinding binding;
        binding.binding = descriptor.binding;
        binding.descriptorCount = 1;
        binding.descriptorType = descriptor.type;
        binding.stageFlags = descriptor.stage;
        binding.pImmutableSamplers = nullptr;
        layoutbindings.push_back(binding);
    }

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 1;

    if (vkCreateDescriptorPool(vulkanDevice, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(layoutbindings.size());
    layoutInfo.pBindings = layoutbindings.data();

    if (vkCreateDescriptorSetLayout(vulkanDevice, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;

    if (vkAllocateDescriptorSets(vulkanDevice, &allocInfo, &descriptorSet) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    std::vector<VkWriteDescriptorSet> descriptorWrites{};

    for (const auto& descriptor : descriptors)
    {
        VkWriteDescriptorSet descriptorwrite;
        descriptorwrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorwrite.dstSet = descriptorSet;
        descriptorwrite.dstBinding = descriptor.binding;
        descriptorwrite.dstArrayElement = 0;
        descriptorwrite.descriptorType = descriptor.type;
        descriptorwrite.descriptorCount = 1;
        if (descriptor.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || descriptor.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
        {
            descriptorwrite.pBufferInfo = &descriptor.bufferInfo;
            descriptorwrite.pImageInfo = nullptr;
            if (descriptor.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
            {
                ++dynamic_count;
                dynamic_offset.push_back(static_cast<uint32_t>(descriptor.bufferInfo.range));
            }
        }
        else
        {
            descriptorwrite.pImageInfo = &descriptor.imageInfo;
            descriptorwrite.pBufferInfo = nullptr;
        }
        descriptorwrite.pTexelBufferView = nullptr;
        descriptorwrite.pNext = nullptr;

        descriptorWrites.push_back(descriptorwrite);
    }

    vkUpdateDescriptorSets(vulkanDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void DescriptorSet::BindDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t offset)
{
    std::vector<uint32_t> memoffset;

    for (auto off : dynamic_offset)
    {
        memoffset.push_back(off * offset);
    }

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
        0, 1, &descriptorSet, dynamic_count, memoffset.data());
}

VkDescriptorSetLayout DescriptorSet::GetSetLayout() const
{
    return descriptorSetLayout;
}
