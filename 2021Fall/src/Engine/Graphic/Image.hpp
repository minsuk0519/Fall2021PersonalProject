#pragma once

//3rd party library
#include <vulkan/vulkan.h>

#include "Buffer.hpp"

enum class ImageType
{
	FRAMEBUFFER = 0,
	TEXTURE = 1,
	SWAPCHAIN = 2,
	MAX
};

class Image
{
public:
	void close();

	friend class VulkanMemoryManager;

public:
	VkImageView GetImageView() const;

private:
	Image(uint32_t width, uint32_t height, ImageType t);

private:
	VkImage image;
	VkDeviceMemory memory;
	VkImageView imageview;

	VkSampler sampler;
	VkFormat format;
	VkExtent2D size;

	ImageType type;
};