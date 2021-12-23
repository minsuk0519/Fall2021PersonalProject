#include "Descriptor.hpp"
#include "Engine/Misc/helper.hpp"
#include "DescriptorSet.hpp"
#include "Graphic.hpp"

//standard library
#include <stdexcept>

DescriptorManager::DescriptorManager(VkDevice device) : vulkanDevice(device) {}

void DescriptorManager::init()
{
	shaders[SHADER_ID_BASERENDER_VERTEX] = { CreateShaderModule("data/shaders/baserendervert.spv"), VK_SHADER_STAGE_VERTEX_BIT,
		{
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0},
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1}
		} };
	shaders[SHADER_ID_BASERENDER_FRAG] = { CreateShaderModule("data/shaders/baserenderfrag.spv"), VK_SHADER_STAGE_FRAGMENT_BIT,
		{
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1}
		} };
	shaders[SHADER_ID_DEFERRED_VERTEX] = { CreateShaderModule("data/shaders/deferredvert.spv"), VK_SHADER_STAGE_VERTEX_BIT, {} };
	shaders[SHADER_ID_DEFERRED_FRAG] = { CreateShaderModule("data/shaders/deferredfrag.spv"), VK_SHADER_STAGE_FRAGMENT_BIT,
		{
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0},
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2},
			{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3},
			{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4},
			{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 5},

			{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6, MAX_LIGHT},
		} };
	shaders[SHADER_ID_DIFFUSE_VERTEX] = { CreateShaderModule("data/shaders/cuberendervert.spv"), VK_SHADER_STAGE_VERTEX_BIT, 
		{
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0},
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1}
		} };
	shaders[SHADER_ID_DIFFUSE_FRAG] = { CreateShaderModule("data/shaders/cuberenderfrag.spv"), VK_SHADER_STAGE_FRAGMENT_BIT,
		{
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1}
		} };
	shaders[SHADER_ID_SHADOWMAP_VERTEX] = { CreateShaderModule("data/shaders/shadowmapvert.spv"), VK_SHADER_STAGE_VERTEX_BIT,
		{
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1}
		} };
	shaders[SHADER_ID_SHADOWMAP_GEOM] = { CreateShaderModule("data/shaders/shadowmapgeom.spv"), VK_SHADER_STAGE_GEOMETRY_BIT,
		{
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 3}
		} };
	shaders[SHADER_ID_SHADOWMAP_FRAG] = { CreateShaderModule("data/shaders/shadowmapfrag.spv"), VK_SHADER_STAGE_FRAGMENT_BIT,
		{
		} };

	programs[PROGRAM_ID::PROGRAM_ID_BASERENDER] = { SHADER_ID_BASERENDER_VERTEX, SHADER_ID_BASERENDER_FRAG };
	programs[PROGRAM_ID::PROGRAM_ID_DEFERRED] = { SHADER_ID_DEFERRED_VERTEX, SHADER_ID_DEFERRED_FRAG };
	programs[PROGRAM_ID::PROGRAM_ID_DIFFUSE] = { SHADER_ID_DIFFUSE_VERTEX, SHADER_ID_DIFFUSE_FRAG };
	programs[PROGRAM_ID::PROGRAM_ID_SHADOWMAP] = { SHADER_ID_SHADOWMAP_VERTEX, SHADER_ID_SHADOWMAP_GEOM, SHADER_ID_SHADOWMAP_FRAG };

	SetupShaderPrograms(programs);
}

void DescriptorManager::close()
{
	vkDestroyDescriptorPool(vulkanDevice, vulkanDescriptorPool, nullptr);

	for (auto setlayout : vulkanDescriptorSetLayouts)
	{
		vkDestroyDescriptorSetLayout(vulkanDevice, setlayout, nullptr);
	}
	vulkanDescriptorSetLayouts.clear();

	for (auto pipelinelayout : vulkanPipelineLayouts)
	{
		vkDestroyPipelineLayout(vulkanDevice, pipelinelayout, nullptr);
	}
	vulkanPipelineLayouts.clear();

	for (auto shader : shaders)
	{
		vkDestroyShaderModule(vulkanDevice, shader.shadermodule, nullptr);
	}
}

void DescriptorManager::SetupShaderPrograms(const std::array<std::vector<SHADER_ID>, PROGRAM_ID::PROGRAM_ID_MAX>& programs)
{
	std::vector<VkDescriptorPoolSize> poolSizes{};

	uint32_t programindex = 0;
	for (auto program : programs)
	{
		std::vector<VkDescriptorSetLayoutBinding> layoutbindings;

		for (auto ID : program)
		{
			VkShaderStageFlags stagebits = shaders[ID].stage;

			VkPipelineShaderStageCreateInfo shaderStageInfo{};
			shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStageInfo.stage = static_cast<VkShaderStageFlagBits>(stagebits);
			shaderStageInfo.module = shaders[ID].shadermodule;
			shaderStageInfo.pName = "main";
			programShaderStageCreateInfo[programindex].push_back(shaderStageInfo);

			for (auto descriptor : shaders[ID].descriptors)
			{
				bool repeated = false;
				for (auto& binding : layoutbindings)
				{
					if (descriptor.binding == binding.binding && descriptor.type == binding.descriptorType)
					{
						binding.stageFlags |= stagebits;
						repeated = true;
						break;
					}
				}

				bool countzero = true;
				for (auto& pool : poolSizes)
				{
					if (pool.type == descriptor.type)
					{
						++pool.descriptorCount;
						countzero = false;
						break;
					}
				}
				if (countzero)
				{
					VkDescriptorPoolSize poolsize;
					poolsize.type = descriptor.type;
					poolsize.descriptorCount = 1;
					poolSizes.push_back(poolsize);
				}

				if (repeated) continue;

				VkDescriptorSetLayoutBinding binding;
				binding.binding = descriptor.binding;
				binding.descriptorCount = descriptor.count;
				binding.descriptorType = descriptor.type;
				binding.stageFlags = stagebits;
				binding.pImmutableSamplers = nullptr;
				layoutbindings.push_back(binding);
				programsDescriptor[programindex].push_back({ descriptor.type, descriptor.binding, descriptor.count });
			}
		}

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(layoutbindings.size());
		layoutInfo.pBindings = layoutbindings.data();

		if (vkCreateDescriptorSetLayout(vulkanDevice, &layoutInfo, nullptr, &setlayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor set layout!");
		}

		vulkanDescriptorSetLayouts.push_back(setlayout);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;// static_cast<uint32_t>(vulkanDescriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = &setlayout;// vulkanDescriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		VkPipelineLayout pipelineLayout;

		if (vkCreatePipelineLayout(vulkanDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create pipelinelayout");
		}

		vulkanPipelineLayouts.push_back(pipelineLayout);

		++programindex;
	}

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = 16; //hardcoded for now

	if (vkCreateDescriptorPool(vulkanDevice, &poolInfo, nullptr, &vulkanDescriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

VkShaderModule DescriptorManager::CreateShaderModule(const char* filename)
{
	VkShaderModule shaderModule;

	std::vector<char> code = Helper::readFile(filename);

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	if (vkCreateShaderModule(vulkanDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

VkPipelineLayout DescriptorManager::GetpipeLineLayout(const PROGRAM_ID& id) const
{
	if (id >= PROGRAM_ID_MAX || id < 0) throw std::runtime_error("the index of pipelineLayout is incorrect!");

	return vulkanPipelineLayouts[id];
}

VkDescriptorPool DescriptorManager::GetdescriptorPool() const
{
	return vulkanDescriptorPool;
}

DescriptorSet* DescriptorManager::CreateDescriptorSet(const PROGRAM_ID& id, const std::vector<DescriptorData> data) const
{
	DescriptorSet* descriptorset = new DescriptorSet(vulkanDevice);

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = vulkanDescriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &(vulkanDescriptorSetLayouts[static_cast<int>(id)]);

	if (vkAllocateDescriptorSets(vulkanDevice, &allocInfo, &descriptorset->descriptorSet) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	std::vector<VkWriteDescriptorSet> descriptorWrites{};

	//if (programsDescriptor[id].size() != data.size()) throw std::runtime_error("the descriptor size of target shader program is different with data size");

	uint32_t dataindex = 0;
	for (auto descriptor : programsDescriptor[id])
	{
		VkWriteDescriptorSet descriptorwrite;
		descriptorwrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorwrite.dstSet = descriptorset->descriptorSet;
		descriptorwrite.dstBinding = descriptor.binding;
		descriptorwrite.descriptorType = descriptor.type;
		descriptorwrite.descriptorCount = 1;
		descriptorwrite.pTexelBufferView = nullptr;
		descriptorwrite.pNext = nullptr;

		for (uint32_t i = 0; i < descriptor.count; ++i)
		{
			descriptorwrite.dstArrayElement = i;
			descriptorwrite.pBufferInfo = (data[dataindex].bufferinfo.has_value() ? &data[dataindex].bufferinfo.value() : nullptr);
			descriptorwrite.pImageInfo = (data[dataindex].imageinfo.has_value() ? &data[dataindex].imageinfo.value() : nullptr);
			if (descriptor.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
			{
				++(descriptorset->dynamic_count);
				descriptorset->dynamic_offset.push_back(static_cast<uint32_t>(descriptorwrite.pBufferInfo->range));
			}
			descriptorWrites.push_back(descriptorwrite);
			++dataindex;
		}
	}

	vkUpdateDescriptorSets(vulkanDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

	return descriptorset;
}

std::vector<VkPipelineShaderStageCreateInfo> DescriptorManager::Getshadermodule(const PROGRAM_ID& id) const
{
	return programShaderStageCreateInfo[id];
}

VkDescriptorSetLayout DescriptorManager::GetdescriptorSetLayout(const PROGRAM_ID& id) const
{
	if (id >= PROGRAM_ID_MAX || id < 0) throw std::runtime_error("the index of pipelineLayout is incorrect!");

	return vulkanDescriptorSetLayouts[id];
}
