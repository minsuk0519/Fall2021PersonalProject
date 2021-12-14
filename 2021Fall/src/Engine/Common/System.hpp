#pragma once
#include "Interface.hpp"

//standard library
#include <string>

//3rd party library
#include <vulkan/vulkan.h>

class Application;

class System : public Interface
{
public:
	System(VkDevice device, Application* app, std::string n) : vulkanDevice(device), application(app), name(n), Interface() {}

	virtual void init() override = 0;
	virtual void postinit() override = 0;

	virtual void update(float dt) override = 0;
	virtual void drawGUI() = 0;
	virtual void close() override = 0;
	virtual ~System() override {};

public:
	std::string name = "";

protected:
	VkDevice vulkanDevice = nullptr;

	Application* application = nullptr;
};