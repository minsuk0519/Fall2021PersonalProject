#pragma once

#include <vulkan/vulkan.h>

class Application;

class System
{
public:
	System(VkDevice device, Application* app) : vulkanDevice(device), application(app) {}

	virtual void init() = 0;
	virtual void update(float dt) = 0;
	virtual void close() = 0;
	virtual ~System() {};

protected:
	VkDevice vulkanDevice = nullptr;

	Application* application = nullptr;
};