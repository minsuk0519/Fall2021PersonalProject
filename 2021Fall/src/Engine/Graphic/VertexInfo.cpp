#include "VertexInfo.hpp"

VkVertexInputBindingDescription PosNormal::getBindingDescription()
{
	VkVertexInputBindingDescription result{};

	result.binding = 0;
	result.stride = sizeof(PosNormal);
	result.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return result;
}

std::array<VkVertexInputAttributeDescription, 2> PosNormal::getAttributeDescriptions()
{
	std::array<VkVertexInputAttributeDescription, 2> result{};

	result[0].binding = 0;
	result[0].location = 0;
	result[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	result[0].offset = offsetof(PosNormal, position);

	result[1].binding = 0;
	result[1].location = 1;
	result[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	result[1].offset = offsetof(PosNormal, normal);

	return result;
}

bool PosNormal::operator==(const PosNormal& rhs) const
{
	return ((position == rhs.position) && (normal == rhs.normal));
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

bool PosColorTexVertex::operator==(const PosColorTexVertex& rhs) const
{
	return ((position == rhs.position) && (color == rhs.color) && (texCoord == rhs.texCoord));
}

VkVertexInputBindingDescription PosTexVertex::getBindingDescription()
{
	VkVertexInputBindingDescription result{};

	result.binding = 0;
	result.stride = sizeof(PosTexVertex);
	result.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return result;
}

std::array<VkVertexInputAttributeDescription, 2> PosTexVertex::getAttributeDescriptions()
{
	std::array<VkVertexInputAttributeDescription, 2> result{};

	result[0].binding = 0;
	result[0].location = 0;
	result[0].format = VK_FORMAT_R32G32_SFLOAT;
	result[0].offset = offsetof(PosTexVertex, position);

	result[1].binding = 0;
	result[1].location = 1;
	result[1].format = VK_FORMAT_R32G32_SFLOAT;
	result[1].offset = offsetof(PosTexVertex, texCoord);

	return result;
}

bool PosTexVertex::operator==(const PosTexVertex& rhs) const
{
	return ((position == rhs.position) && (texCoord == rhs.texCoord));
}
