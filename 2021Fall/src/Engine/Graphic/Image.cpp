#include "Image.hpp"

void Image::close()
{
	if (type == ImageType::SWAPCHAIN)
	{
		VulkanMemoryManager::FreeImage(nullptr, imageview, memory);
	}
	else
	{
		VulkanMemoryManager::FreeImage(image, imageview, memory);
	}
}

VkImageView Image::GetImageView() const
{
	return imageview;
}

Image::Image(uint32_t width, uint32_t height, ImageType t)
{
	size.width = width;
	size.height = height;
	type = t;
}