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
	RENDERPASS_MAX = DEPTHATTACHMENT + 1,
};

class Renderpass;
class DescriptorSet;
class Buffer;
class Image;
class Camera;

class Graphic : public System
{
public:
	Graphic(VkDevice device, Application* app);

	void init() override;
	void update(float dt) override;
	void close() override;
	~Graphic() override;

private:
	VkCommandBuffer vulkanCommandBuffers;
	std::vector<VkSemaphore> vulkanImageAvailableSemaphores;
	std::vector<VkSemaphore> vulkanRenderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	std::vector<VkFence> imagesInFlight;

	VkSwapchainKHR vulkanSwapChain;

	VkExtent2D vulkanSwapChainExtent;

	VkSampler vulkanTextureSampler;

	//should be moved later?
	Camera* camera = nullptr;

private:
	VkSampleCountFlagBits vulkanMSAASamples = VK_SAMPLE_COUNT_1_BIT;

	size_t currentFrame = 0;

	uint32_t textureMipLevels;

	GraphicPipeline* graphicPipeline = nullptr;
	Renderpass* renderpass = nullptr;
	DescriptorSet* descriptorSet = nullptr;

	Renderpass* postrenderpass = nullptr;
	DescriptorSet* postdescriptorSet = nullptr;
	GraphicPipeline* postgraphicPipeline = nullptr;

	std::vector<VkCommandBuffer> vulkanpostCommandBuffer;

	std::vector<Buffer*> buffers;
	std::vector<Image*> images;

	std::vector<Image*> swapchainImages;
	std::vector<Image*> framebufferImages;
	uint32_t swapchainImageSize;
	VkFormat vulkanSwapChainImageFormat;
	VkFormat vulkanDepthFormat;

private:
	void SetupSwapChain();
	void DefineDrawBehavior();
	void CloseSwapChain();
	void RecreateSwapChain();

	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	void loadModel(tinyobj::attrib_t& attrib, std::vector<tinyobj::shape_t>& shapes, const char* path);

	VkSampleCountFlagBits getMaxUsableSampleCount();
};

#include <glm/mat4x4.hpp>

struct transform
{
	glm::mat4 worldToCamera;
	glm::mat4 cameraToNDC;
	glm::mat4 objectMat;
};