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

VkVertexInputBindingDescription PosColorTexVertex::getBindingDescription()
{
	VkVertexInputBindingDescription result{};

	result.binding = 0;
	result.stride = sizeof(PosColorTexVertex);
	result.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return result;
}

std::array<VkVertexInputAttributeDescription, 3> PosColorTexVertex::getAttributeDescriptions()
{
	std::array<VkVertexInputAttributeDescription, 3> result{};

	result[0].binding = 0;
	result[0].location = 0;
	result[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	result[0].offset = offsetof(PosColorTexVertex, position);

	result[1].binding = 0;
	result[1].location = 1;
	result[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	result[1].offset = offsetof(PosColorTexVertex, color);

	result[2].binding = 0;
	result[2].location = 2;
	result[2].format = VK_FORMAT_R32G32_SFLOAT;
	result[2].offset = offsetof(PosColorTexVertex, texCoord);

	return result;
}
