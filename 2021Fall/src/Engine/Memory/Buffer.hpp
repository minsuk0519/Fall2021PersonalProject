#pragma once

//3rd party library
#include <vulkan/vulkan.h>

//standard library
#include <vector>

enum BUFFERTYPE
{
	BUFFER_VERTEX,
	BUFFER_INDEX,
	BUFFER_UNIFORM,
	BUFFER_MAX,
};

enum UniformBufferIndex
{
	UNIFORM_CAMERA_TRANSFORM = 0,
	UNIFORM_OBJECT_MATRIX,
	UNIFORM_LIGHT_OBJECT_MATRIX,
	UNIFORM_GUI_SETTING,
	UNIFORM_LIGHTDATA,
	UNIFORM_LIGHTPROJ,
	UNIFORM_BUFFER_MAX
};

class Buffer;
class Image;

class VulkanMemoryManager
{
public:
	static void Init(VkDevice device);
	static void Close();

	static uint32_t CreateVertexBuffer(void* memory, size_t memorysize);
	static uint32_t CreateIndexBuffer(void* memory, size_t memorysize);
	static uint32_t CreateUniformBuffer(UniformBufferIndex index, size_t memorysize, uint32_t num = 1);
	
	static void GetSwapChainImage(VkSwapchainKHR swapchain, uint32_t& imagecount, std::vector<Image*>& images, const VkFormat& format);
	static Image* CreateFrameBufferImage(VkImageUsageFlags usage, VkFormat format, VkSampleCountFlagBits sample);
	static Image* CreateDepthBuffer(VkFormat format, VkSampleCountFlagBits sample);
	static Image* CreateShadowMapBuffer();
	static Image* CreateTextureImage(int width, int height, unsigned char* pixels);

	static Buffer* GetBuffer(uint32_t index);
	static Buffer* GetUniformBuffer(UniformBufferIndex index);

private:
	static VkDevice vulkanDevice;
	static VkPhysicalDevice vulkanPhysicalDevice;
	static VkQueue vulkanQueue;
	static VkCommandPool vulkanCommandpool;

	static std::vector<Buffer*> buffers;
	static uint32_t bufferIndex;

	static std::vector<uint32_t> uniformIndices;

public:
	static void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
		VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	static void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	static void createImage(uint32_t width, uint32_t height, uint32_t layer, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling,
		VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageCreateFlags flag, VkImage& image, VkDeviceMemory& imageMemory);

	static VkCommandBuffer beginSingleTimeCommands();
	static void endSingleTimeCommands(VkCommandBuffer commandBuffer);
	static void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layercount, uint32_t mipLevels);
	static void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	static VkImageView createImageView(VkImage image, VkFormat format, uint32_t layercount, VkImageViewType viewtype, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
	static void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

	static void FreeBuffer(VkBuffer buf, VkDeviceMemory devicememory);
	static void FreeImage(VkImage image, VkImageView imageview, VkDeviceMemory devicememory);

	static void MapMemory(VkDeviceMemory devicememory, size_t size, void* data);

	static void MapMemory(uint32_t index, void* data, size_t size = 0, uint32_t offset = 0);
};

class Buffer
{
public:
	void close();

	friend class VulkanMemoryManager;
public:
	VkBuffer GetBuffer() const;
	VkDeviceMemory GetMemory() const;

	VkDescriptorBufferInfo GetDescriptorInfo() const;

private:
	Buffer();

private:
	uint32_t id;

	BUFFERTYPE type;
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory memory;
	VkDeviceSize size;
	VkDeviceSize offset;
};