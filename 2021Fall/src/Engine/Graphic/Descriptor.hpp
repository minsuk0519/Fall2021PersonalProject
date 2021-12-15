#pragma once

//3rd party library
#include <vulkan/vulkan.h>

//standard library
#include <vector>
#include <array>
#include <optional>

enum SHADER_ID
{
	SHADER_ID_BASERENDER_VERTEX,
	SHADER_ID_BASERENDER_FRAG,
	SHADER_ID_DEFERRED_VERTEX,
	SHADER_ID_DEFERRED_FRAG,
	SHADER_ID_DIFFUSE_VERTEX,
	SHADER_ID_DIFFUSE_FRAG,
	SHADER_ID_MAX,
};

enum PROGRAM_ID
{
	PROGRAM_ID_BASERENDER = 0,
	PROGRAM_ID_DEFERRED = 1,
	PROGRAM_ID_DIFFUSE = 2,
	PROGRAM_ID_MAX,
};

struct Descriptor
{
	VkDescriptorType type;
	uint32_t binding;
};

struct Shader
{
	VkShaderModule shadermodule;

	VkShaderStageFlags stage;
	std::vector<Descriptor> descriptors;
};

struct DescriptorData
{
	std::optional<VkDescriptorBufferInfo> bufferinfo;
	std::optional<VkDescriptorImageInfo> imageinfo;
};

struct Program
{
	std::vector<VkShaderModule> shadermodule;
};

class DescriptorSet;

class DescriptorManager
{
public:
	DescriptorManager(VkDevice device);

	void init();
	void close();

	VkPipelineLayout GetpipeLineLayout(const PROGRAM_ID& id) const;
	VkDescriptorSetLayout GetdescriptorSetLayout(const PROGRAM_ID& id) const;

	VkDescriptorPool GetdescriptorPool() const;

	DescriptorSet* CreateDescriptorSet(const PROGRAM_ID& id, const std::vector<DescriptorData> data) const;

	std::vector<VkPipelineShaderStageCreateInfo> Getshadermodule(const PROGRAM_ID& id) const;
private:
	VkDescriptorSetLayout setlayout;

	std::vector<VkDescriptorSetLayout> vulkanDescriptorSetLayouts;
	std::vector<VkPipelineLayout> vulkanPipelineLayouts;

	VkDescriptorPool vulkanDescriptorPool;
	std::array<Shader, SHADER_ID_MAX> shaders;
	std::array<std::vector<Descriptor>, PROGRAM_ID_MAX> programsDescriptor;

	std::array<std::vector<VkPipelineShaderStageCreateInfo>, PROGRAM_ID_MAX> programShaderStageCreateInfo;

	std::array<std::vector<SHADER_ID>, PROGRAM_ID::PROGRAM_ID_MAX> programs;

	VkDevice vulkanDevice;
	
	void SetupShaderPrograms(const std::array<std::vector<SHADER_ID>, PROGRAM_ID::PROGRAM_ID_MAX>& programs);

	VkShaderModule CreateShaderModule(const char* filename);
};