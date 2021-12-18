#include "DescriptorSet.hpp"
#include "Descriptor.hpp"

//standard library
#include <stdexcept>
#include <array>

DescriptorSet::DescriptorSet(VkDevice device) : vulkanDevice(device) {}

void DescriptorSet::init()
{
}

void DescriptorSet::close()
{
}

void DescriptorSet::BindDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, std::vector<uint32_t> offset)
{
    std::vector<uint32_t> memoffset;

    if (offset.size() != dynamic_offset.size())
    {
        throw std::runtime_error("offset index is not correct!");
    }

    for (uint32_t i = 0; i < offset.size(); ++i)
    {
        memoffset.push_back(offset[i] * dynamic_offset[i]);
    }

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
        0, 1, &descriptorSet, dynamic_count, memoffset.data());
}