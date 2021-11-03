#pragma once

//standard library
#include <array>

//3rd party librarys
#include <tinyobjloader/tiny_obj_loader.h>

#include "Engine/System.hpp"
#include "GraphicPipeline.hpp"

enum RENDERPASS
{
	COLORATTACHMENT = 0,
	NORMALATTACHMENT,
	POSITIONATTACHMENT,
	COLORATTACHMENT_MAX,
	COLORATTACHMENT_MSAA = COLORATTACHMENT + COLORATTACHMENT_MAX,
	NORMALATTACHMENT_MSAA = NORMALATTACHMENT + COLORATTACHMENT_MAX,
	POSITIONATTACHMENT_MSAA = POSITIONATTACHMENT + COLORATTACHMENT_MAX,

	DEPTHATTACHMENT = COLORATTACHMENT_MAX * 2,
	FINALIMAGE = DEPTHATTACHMENT + 1,
	RENDERPASS_MAX = FINALIMAGE,
};

class Renderpass;

class Graphic : public System
{
public:
	Graphic(VkDevice device, Application* app);

	void init() override;
	void update(float dt) override;
	void close() override;
	~Graphic() override;

private:
	//VkRenderPass vulkanRenderPass;

	std::vector<VkCommandBuffer> vulkanCommandBuffers;

	//std::vector<VkFramebuffer> vulkanSwapChainFramebuffers;
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

	std::array<VkImage, RENDERPASS::DEPTHATTACHMENT> vulkanColorImage;
	std::array<VkDeviceMemory, RENDERPASS::DEPTHATTACHMENT> vulkanColorImageMemory;
	std::array<VkImageView, RENDERPASS::DEPTHATTACHMENT> vulkanColorImageView;

	std::vector<VkDeviceMemory> vulkanDeviceMemories;
	std::vector<VkBuffer> vulkanBuffers;

private:
	GraphicPipeline* graphicPipeline = nullptr;

	VkSampleCountFlagBits vulkanMSAASamples = VK_SAMPLE_COUNT_1_BIT;

	size_t currentFrame = 0;

	uint32_t textureMipLevels;

	Renderpass* renderpass = nullptr;


private:
	void SetupSwapChain();
	void DefineDrawBehavior();
	void CloseSwapChain();
	void RecreateSwapChain();

	VkBuffer createVertexBuffer(void* memory, size_t memorysize);
	VkBuffer createIndexBuffer(void* memory, size_t memorysize);

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
		VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling,
		VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	void loadModel(tinyobj::attrib_t& attrib, std::vector<tinyobj::shape_t>& shapes, const char* path);
	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

	VkSampleCountFlagBits getMaxUsableSampleCount();
};

#include <glm/mat4x4.hpp>

struct transform
{
	glm::mat4 worldToCamera;
	glm::mat4 cameraToNDC;
	glm::mat4 objectMat;
};