#pragma once

//standard library
#include <array>

//3rd party library
#include <glm/vec3.hpp>
#include <vulkan/vulkan.h>

struct PosColorVertex
{
	glm::vec3 position;
	glm::vec3 color;

	static VkVertexInputBindingDescription getBindingDescription();

	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
};