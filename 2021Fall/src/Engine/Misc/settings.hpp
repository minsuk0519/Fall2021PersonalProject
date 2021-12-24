#pragma once

//3rd party library
#include <vulkan/vulkan.h>

namespace Settings
{
	extern unsigned int windowWidth;
	extern unsigned int windowHeight;

	extern unsigned int shadowmapSize;

	float GetAspectRatio();
};
