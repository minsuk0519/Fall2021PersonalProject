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

	VkBuffer vulkanVertexBuffer;
	VkDeviceMemory vulkanVertexBufferMemory;

	VkBuffer vulkanIndexBuffer;
	VkDeviceMemory vulkanIndexBufferMemory;

	std::vector<VkBuffer> vulkanUniformBuffers;
	std::vector<VkDeviceMemory> vulkanUniformBuffersMemory;

	VkDescriptorSetLayout vulkanDescriptorSetLayout;
	VkDescriptorPool vulkanDescriptorPool;
	std::vector<VkDescriptorSet> vulkanDescriptorSets;

	VkImage vulkanTextureImage;
	VkDeviceMemory vulkanTextureImageMemory;
	VkImageView vulkanTextureImageView;
	VkSampler vulkanTextureSampler;

	VkImage vulkanDepthImage;
	VkDeviceMemory vulkanDepthImageMemory;
	VkImageView vulkanDepthImageView;
	VkFormat vulkanDepthFormat;

	size_t currentFrame = 0;

private:
	void SetupSwapChain();
	void DefineDrawBehavior();
	void CloseSwapChain();
	void RecreateSwapChain();

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
		VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
		VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
};

#include <glm/mat4x4.hpp>

struct transform
{
	glm::mat4 worldToCamera;
	glm::mat4 cameraToNDC;
	glm::mat4 objectMat;
};