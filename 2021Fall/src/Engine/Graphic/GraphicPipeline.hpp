#pragma once
#include "ShaderModule.hpp"

class GraphicPipeline
{
public:
	GraphicPipeline(VkDevice device);

	void init();
	void close();
private:
	ShaderModule* vertShaderModule = nullptr;
	ShaderModule* fragShaderModule = nullptr;

	VkDevice vulkanDevice;
};