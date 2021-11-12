#pragma once

//standard library
#include <string>

//3rd party library
#include <vulkan/vulkan.h>

class Application;

class System
{
public:
	System(VkDevice device, Application* app, std::string n) : vulkanDevice(device), application(app), name(n) {}

	virtual void init() = 0;
	virtual void update(float dt) = 0;
	virtual void drawGUI() = 0;
	virtual void close() = 0;
	virtual ~System() {};

public:
	std::string name = "";

protected:
	VkDevice vulkanDevice = nullptr;

	Application* application = nullptr;
};