#pragma once

#include <vulkan/vulkan.h>

class System
{
public:
	System(VkDevice device) : vulkanDevice(device) {}

	virtual void init() = 0;
	virtual void update(float dt) = 0;
	virtual void close() = 0;

protected:
	VkDevice vulkanDevice = nullptr;
};