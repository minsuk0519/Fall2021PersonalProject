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
	~Graphic() override;

private:
	GraphicPipeline* graphicPipeline = nullptr;

	VkRenderPass vulkanRenderPass;

	std::vector<VkCommandBuffer> vulkanCommandBuffers;

	std::vector<VkFramebuffer> vulkanSwapChainFramebuffers;
	std::vector<VkSemaphore> vulkanImageAvailableSemaphores;
	std::vector<VkSemaphore> vulkanRenderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	std::vector<VkFence> imagesInFlight;

	std::vector<VkImageView> vulkanSwapChainImageViews;

	VkFormat vulkanSwapChainImageFormat;

	VkSwapchainKHR vulkanSwapChain;

	std::vector<VkImage> vulkanSwapChainImages;

	VkExtent2D vulkanSwapChainExtent;

	size_t currentFrame = 0;

	void SetupSwapChain();
	void CloseSwapChain();
	void RecreateSwapChain();
};