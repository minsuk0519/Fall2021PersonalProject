#pragma once
//third party library
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

class Application
{
public:
	bool init();
	bool update();
	bool close();

private:
	GLFWwindow* window = nullptr;

	VkInstance vulkanInstance;
};