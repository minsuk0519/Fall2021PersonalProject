#pragma once

//standard library
#include <array>
#include <optional>

//3rd party librarys
#include <tinyobjloader/tiny_obj_loader.h>
#include <glm/vec3.hpp>

#include "Engine/System.hpp"
#include "GraphicPipeline.hpp"

enum FrameBufferIndex
{
	NORMALATTACHMENT,
	POSITIONATTACHMENT,
	COLORATTACHMENT_MAX,
	NORMALATTACHMENT_MSAA = NORMALATTACHMENT + COLORATTACHMENT_MAX,
	POSITIONATTACHMENT_MSAA = POSITIONATTACHMENT + COLORATTACHMENT_MAX,

	DEPTHATTACHMENT = COLORATTACHMENT_MAX * 2,
	FRAMEBUFFER_MAX = DEPTHATTACHMENT + 1,
};

enum UniformBufferIndex
{
	UNIFORM_CAMERA_TRANSFORM = 0,
	UNIFORM_OBJECT_MATRIX,
	UNIFORM_GUI_SETTING,
	UNIFORM_BUFFER_MAX
};

class Renderpass;
class DescriptorSet;
class Buffer;
class Image;
class Camera;

struct GUISetting
{
	int deferred_type = 0;
};

struct VertexInfo
{
	uint32_t vertex;
	uint32_t index;

	uint32_t indexSize;
};

struct DrawTarget
{
	std::vector<VertexInfo> vertexIndices;

	std::optional<uint32_t> instancebuffer;
	std::optional<uint32_t> instancenumber;

	void AddVertex(VertexInfo info);
};

class Graphic : public System
{
public:
	Graphic(VkDevice device, Application* app);

	void init() override;
	void update(float dt) override;
	void close() override;
	~Graphic() override;
	void drawGUI() override;

private:
	VkCommandBuffer vulkanCommandBuffers;
	std::vector<VkSemaphore> vulkanImageAvailableSemaphores;
	std::vector<VkSemaphore> vulkanRenderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	std::vector<VkFence> imagesInFlight;

	VkSwapchainKHR vulkanSwapChain;

	VkExtent2D vulkanSwapChainExtent;

	VkSampler vulkanTextureSampler;
	VkFormat vulkanSwapChainImageFormat;
	VkFormat vulkanDepthFormat;

	//should be moved later?
	Camera* camera = nullptr;
	glm::vec3 position[3] = { glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) };

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

	std::vector<DrawTarget> drawtargets;

	std::vector<Image*> swapchainImages;
	std::vector<Image*> framebufferImages;
	std::vector<Image*> images;
	uint32_t swapchainImageSize;

	GUISetting guiSetting;

	std::vector<uint32_t> uniformBuffers;

private:
	void SetupSwapChain();
	void DefineDrawBehavior();
	void CloseSwapChain();
	void RecreateSwapChain();

	void DrawDrawtarget(const VkCommandBuffer& cmdBuffer, const DrawTarget& target);

	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	void loadModel(tinyobj::attrib_t& attrib, std::vector<tinyobj::shape_t>& shapes, const std::string& path, const std::string& filename);

	VkSampleCountFlagBits getMaxUsableSampleCount();
};