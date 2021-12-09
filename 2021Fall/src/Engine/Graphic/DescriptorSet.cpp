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