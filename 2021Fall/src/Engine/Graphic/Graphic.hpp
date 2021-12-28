#pragma once

//standard library
#include <array>
#include <optional>
#include <vector>
#include <unordered_map>

//3rd party librarys
#include <tinyobjloader/tiny_obj_loader.h>
#include <glm/vec3.hpp>

#include "Engine/Common/System.hpp"
#include "GraphicPipeline.hpp"
#include "Engine/Misc/GUIEnum.hpp"
#include "Descriptor.hpp"
#include "Engine/Memory/Buffer.hpp"

//defined in common.glsl
#define MAX_LIGHT 8

enum FrameBufferIndex
{
	NORMALATTACHMENT,
	POSITIONATTACHMENT,
	ALBEDOATTACHMENT,
	COLORATTACHMENT_MAX,
	NORMALATTACHMENT_MSAA = NORMALATTACHMENT + COLORATTACHMENT_MAX,
	POSITIONATTACHMENT_MSAA = POSITIONATTACHMENT + COLORATTACHMENT_MAX,
	ALBEDOATTACHMENT_MSAA = ALBEDOATTACHMENT + COLORATTACHMENT_MAX,

	DEPTHATTACHMENT = COLORATTACHMENT_MAX * 2,
	FRAMEBUFFER_MAX = DEPTHATTACHMENT + 1,
};

enum DRAWTARGET_INDEX
{
	DRAWTARGET_RECTANGLE,
	DRAWTARGET_MODEL_INSTANCE,
	DRAWTARGET_CUBE,
};

enum RENDERPASS_INDEX
{
	RENDERPASS_POST = 0,
	RENDERPASS_PRE = 1,
	RENDERPASS_DEPTHCUBEMAP = 2,
	RENDERPASS_MAX = RENDERPASS_DEPTHCUBEMAP + 1,
};

enum DESCRIPTORSET_INDEX
{
	//will be same
	DESCRIPTORSET_ID_OBJ = 0,
	DESCRIPTORSET_ID_LIGHT_OBJ = 1,
	DESCRIPTORSET_ID_DEFERRED = 2,
	DESCRIPTORSET_ID_SHADOWMAP = 3,
	DESCRIPTORSET_ID_MAX = 4,
};

enum CMD_INDEX
{
	CMD_BASE = 0,
	CMD_SHADOW = 1,
	CMD_POST = 2,
};

class Renderpass;
class DescriptorSet;
class Buffer;
class Image;
class Camera;
class Light;
class Object;
class DescriptorManager;

struct GUISetting
{
	GUI_ENUM::DEFERRED_TYPE deferred_type = GUI_ENUM::DEFERRED_LIGHT;
	GUI_ENUM::LIGHT_COMPUTATION_TYPE computation_type = GUI_ENUM::LIGHT_COMPUTE_PBR;

	float shadowbias = 0.5f;
	float shadowfar_plane = 100.0f;
	float shadowdiskRadius = 50.0f;
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

struct DrawInfo
{
	void* uniformdata;
	uint32_t uniformsize;
	uint32_t uniformoffset;
};

class Graphic : public System
{
public:
	Graphic(VkDevice device, Application* app);

	void init() override;
	void postinit() override;

	void update(float dt) override;
	void close() override;
	~Graphic() override;
	void drawGUI() override;

public:
	void RegisterObject(DESCRIPTORSET_INDEX descriptorsetid, PROGRAM_ID programid, DRAWTARGET_INDEX drawtargetid);
	void RegisterObject(DESCRIPTORSET_INDEX descriptorsetid, PROGRAM_ID programid, DRAWTARGET_INDEX drawtargetid, std::vector<uint32_t> indices);

	void AddDrawInfo(DrawInfo drawinfo, UniformBufferIndex uniformid);

	void BeginCmdBuffer(CMD_INDEX cmdindex);
	void BeginRenderPass(CMD_INDEX cmdindex, RENDERPASS_INDEX renderpassindex, uint32_t framebufferindex = 0);
	void EndCmdBuffer(CMD_INDEX cmdindex);
	void EndRenderPass(CMD_INDEX cmdindex);

private:
	std::vector<VkCommandBuffer> vulkanCommandBuffers;
	std::vector<VkSemaphore> vulkanImageAvailableSemaphores;
	std::vector<VkSemaphore> vulkanRenderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	std::vector<VkFence> imagesInFlight;

	VkSwapchainKHR vulkanSwapChain;

	VkExtent2D vulkanSwapChainExtent;

	VkSampler vulkanTextureSampler;
	VkFormat vulkanSwapChainImageFormat;
	VkFormat vulkanDepthFormat;

private:
	VkSampleCountFlagBits vulkanMSAASamples = VK_SAMPLE_COUNT_1_BIT;

	size_t currentFrame = 0;

	uint32_t textureMipLevels;

	std::array<GraphicPipeline*, PROGRAM_ID::PROGRAM_ID_MAX> graphicPipelines;
	std::array<DescriptorSet*, DESCRIPTORSET_INDEX::DESCRIPTORSET_ID_MAX> descriptorSets;

	std::array<Renderpass*, RENDERPASS_INDEX::RENDERPASS_MAX> renderPasses;

	std::vector<DrawTarget> drawtargets;

	std::vector<Image*> swapchainImages;
	std::vector<Image*> framebufferImages;
	std::vector<Image*> images;
	uint32_t swapchainImageSize;

	GUISetting guiSetting;
	DescriptorManager* descriptorManager = nullptr;

	std::unordered_map<UniformBufferIndex, std::vector<DrawInfo>> drawinfos;

	CMD_INDEX currentCommandIndex;

private:
	void AllocateCommandBuffer();

	void SetupSwapChain();
	void DefineDrawBehavior();

	void DefineShadowMap();
	void DefinePostProcess();

	void CloseSwapChain();
	void RecreateSwapChain();

	void DrawDrawtarget(const VkCommandBuffer& cmdBuffer, const DrawTarget& target);

	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	void loadModel(tinyobj::attrib_t& attrib, std::vector<tinyobj::shape_t>& shapes, const std::string& path, const std::string& filename);

	VkSampleCountFlagBits getMaxUsableSampleCount();
};