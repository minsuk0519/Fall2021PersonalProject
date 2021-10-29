#pragma once

//3rd party library
#include <vulkan/vulkan.h>

struct Settings
{
public:
	static unsigned int windowWidth;
	static unsigned int windowHeight;
	static float GetAspectRatio();
};
