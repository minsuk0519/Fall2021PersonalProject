#pragma once

#include "Engine/System.hpp"
#include "GraphicPipeline.hpp"

class Graphic : public System
{
public:
	Graphic(VkDevice device, Application* app);

	void init() override;
	void update(float dt) override;
	void close() override;


private:
	GraphicPipeline* graphicPipeline = nullptr;

	VkRenderPass vulkanRenderPass;

	std::vector<VkCommandBuffer> commandBuffers;

	std::vector<VkFramebuffer> vulkanSwapChainFramebuffers;
};