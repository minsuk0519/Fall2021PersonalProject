#pragma once

//standard library
#include <array>

//3rd party library
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <vulkan/vulkan.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

struct PosNormal
{
	glm::vec3 position;
	glm::vec3 normal;

	static VkVertexInputBindingDescription getBindingDescription();

	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();

	bool operator==(const PosNormal& rhs) const;
};

struct PosColorTexVertex
{
	glm::vec3 position;
	glm::vec3 color;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription getBindingDescription();

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();

	bool operator==(const PosColorTexVertex& rhs) const;
};

struct PosTexVertex
{
	glm::vec2 position;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription getBindingDescription();

	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();

	bool operator==(const PosTexVertex& rhs) const;
};

namespace std
{
	template<> struct hash<PosColorTexVertex>
	{
		size_t operator()(PosColorTexVertex const& vertex) const
		{
			return ((hash<glm::vec3>()(vertex.position) ^ 
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};

	template<> struct hash<PosNormal>
	{
		size_t operator()(PosNormal const& vertex) const
		{
			return ((hash<glm::vec3>()(vertex.position) ^
				(hash<glm::vec3>()(vertex.normal) << 1)) >> 1);
		}
	};
}