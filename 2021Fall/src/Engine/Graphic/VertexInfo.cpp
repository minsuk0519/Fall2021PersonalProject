#include "VertexInfo.hpp"

VkVertexInputBindingDescription PosColorVertex::getBindingDescription()
{
	VkVertexInputBindingDescription result{};

	result.binding = 0;
	result.stride = sizeof(PosColorVertex);
	result.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return result;
}

std::array<VkVertexInputAttributeDescription, 2> PosColorVertex::getAttributeDescriptions()
{
	std::array<VkVertexInputAttributeDescription, 2> result{};

	result[0].binding = 0;
	result[0].location = 0;
	result[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	result[0].offset = offsetof(PosColorVertex, position);

	result[1].binding = 0;
	result[1].location = 1;
	result[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	result[1].offset = offsetof(PosColorVertex, color);

	return result;
}
