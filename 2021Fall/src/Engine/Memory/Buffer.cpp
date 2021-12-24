#include "Buffer.hpp"
#include "Engine/Common/Application.hpp"
#include "Engine/Misc/settings.hpp"
#include "Image.hpp"

//standard library
#include <cstring>
#include <stdexcept>
#include <cmath>

VkDevice VulkanMemoryManager::vulkanDevice = VK_NULL_HANDLE;
VkPhysicalDevice VulkanMemoryManager::vulkanPhysicalDevice = VK_NULL_HANDLE;
VkQueue VulkanMemoryManager::vulkanQueue = VK_NULL_HANDLE;
VkCommandPool VulkanMemoryManager::vulkanCommandpool = VK_NULL_HANDLE;

std::vector<Buffer*> VulkanMemoryManager::buffers;
uint32_t VulkanMemoryManager::bufferIndex = 0;

std::vector<uint32_t> VulkanMemoryManager::uniformIndices;

void Buffer::close()
{
    VulkanMemoryManager::FreeBuffer(buffer, memory);
}

VkBuffer Buffer::GetBuffer() const
{
    return buffer;
}

VkDeviceMemory Buffer::GetMemory() const
{
    return memory;
}

uint32_t VulkanMemoryManager::CreateVertexBuffer(void* memory, size_t memorysize)
{
    VkDeviceMemory buffermemory;
    VkBuffer buffer;

    VkBuffer vertexstagingBuffer;
    VkDeviceMemory vertexstagingBufferMemory;

    VkDeviceSize vertexbufferSize = memorysize;
    createBuffer(vertexbufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexstagingBuffer, vertexstagingBufferMemory);

    void* vertexdata;
    vkMapMemory(vulkanDevice, vertexstagingBufferMemory, 0, vertexbufferSize, 0, &vertexdata);
    memcpy(vertexdata, memory, memorysize);
    vkUnmapMemory(vulkanDevice, vertexstagingBufferMemory);

    createBuffer(vertexbufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, buffermemory);

    copyBuffer(vertexstagingBuffer, buffer, vertexbufferSize);

    vkDestroyBuffer(vulkanDevice, vertexstagingBuffer, nullptr);
    vkFreeMemory(vulkanDevice, vertexstagingBufferMemory, nullptr);

    Buffer* buf = new Buffer();

    buf->memory = buffermemory;
    buf->buffer = buffer;
    buf->size = vertexbufferSize;
    buf->type = BUFFERTYPE::BUFFER_VERTEX;
    buf->offset = vertexbufferSize;

    buffers.push_back(buf);

    return bufferIndex++;
}

uint32_t VulkanMemoryManager::CreateIndexBuffer(void* memory, size_t memorysize)
{
    VkDeviceMemory buffermemory;
    VkBuffer buffer;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    VkDeviceSize bufferSize = memorysize;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(vulkanDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, memory, memorysize);
    vkUnmapMemory(vulkanDevice, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, buffermemory);

    copyBuffer(stagingBuffer, buffer, bufferSize);

    vkDestroyBuffer(vulkanDevice, stagingBuffer, nullptr);
    vkFreeMemory(vulkanDevice, stagingBufferMemory, nullptr);

    Buffer* buf = new Buffer();

    buf->memory = buffermemory;
    buf->buffer = buffer;
    buf->size = bufferSize;
    buf->type = BUFFERTYPE::BUFFER_INDEX;
    buf->offset = bufferSize;

    buffers.push_back(buf);

    return bufferIndex++;
}

uint32_t VulkanMemoryManager::CreateUniformBuffer(UniformBufferIndex index, size_t memorysize, uint32_t num)
{
    VkBuffer buffer;
    VkDeviceMemory buffermemory;

    VkDeviceSize totalSize = memorysize * num;

    createBuffer(totalSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, buffermemory);

    Buffer* buf = new Buffer();

    buf->memory = buffermemory;
    buf->buffer = buffer;
    buf->size = totalSize;
    buf->type = BUFFERTYPE::BUFFER_UNIFORM;
    buf->offset = memorysize;

    buffers.push_back(buf);

    uniformIndices[index] = bufferIndex;

    return bufferIndex++;
}

void VulkanMemoryManager::GetSwapChainImage(VkSwapchainKHR swapchain, uint32_t& imagecount, std::vector<Image*>& images, const VkFormat& format)
{
    std::vector<VkImage> swapchainimages;

    vkGetSwapchainImagesKHR(vulkanDevice, swapchain, &imagecount, nullptr);
    swapchainimages.resize(imagecount);
    vkGetSwapchainImagesKHR(vulkanDevice, swapchain, &imagecount, swapchainimages.data());

    for (size_t i = 0; i < imagecount; i++)
    {
        Image* image = new Image(Settings::windowWidth, Settings::windowHeight, ImageType::SWAPCHAIN);

        image->format = format;
        image->image = swapchainimages[i];
        image->imageview = VulkanMemoryManager::createImageView(swapchainimages[i], format, 1, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 1);

        images.push_back(image);
    }
}

Image* VulkanMemoryManager::CreateFrameBufferImage(VkImageUsageFlags usage, VkFormat format, VkSampleCountFlagBits sample)
{
    Image* image = new Image(Settings::windowWidth, Settings::windowHeight, ImageType::FRAMEBUFFER);

    VulkanMemoryManager::createImage(Settings::windowWidth, Settings::windowHeight, 1, 1, sample, format,
        VK_IMAGE_TILING_OPTIMAL, usage,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, image->image, image->memory);

    image->imageview = VulkanMemoryManager::createImageView(image->image, format, 1, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 1);

    image->format = format;

    return image;
}

Image* VulkanMemoryManager::CreateDepthBuffer(VkFormat format, VkSampleCountFlagBits sample)
{
    Image* image = new Image(Settings::windowWidth, Settings::windowHeight, ImageType::FRAMEBUFFER);

    VulkanMemoryManager::createImage(Settings::windowWidth, Settings::windowHeight, 1, 1, sample, format, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, image->image, image->memory);

    image->imageview = VulkanMemoryManager::createImageView(image->image, format, 1, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

    VulkanMemoryManager::transitionImageLayout(image->image, format, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1, 1);

    image->format = format;

    return image;
}

Image* VulkanMemoryManager::CreateShadowMapBuffer()
{
    const uint32_t depthsize = Settings::shadowmapSize;
    const VkFormat depthFormat = VK_FORMAT_D16_UNORM;

    Image* image = new Image(depthsize, depthsize, ImageType::FRAMEBUFFER);

    VulkanMemoryManager::createImage(depthsize, depthsize, 6, 1, VK_SAMPLE_COUNT_1_BIT, depthFormat, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT, image->image, image->memory);

    image->imageview = VulkanMemoryManager::createImageView(image->image, depthFormat, 6, VK_IMAGE_VIEW_TYPE_CUBE, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

    VulkanMemoryManager::transitionImageLayout(image->image, VK_FORMAT_D16_UNORM, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 6, 1);

    image->format = depthFormat;

    return image;
}

Image* VulkanMemoryManager::CreateTextureImage(int width, int height, unsigned char* pixels)
{
    Image* image = new Image(width, height, ImageType::TEXTURE);

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    uint32_t textureMipLevels = static_cast<uint32_t>(std::floor(std::log2(max(width, height)))) + 1;
    VkDeviceSize imageSize = width * height * 4;
    VulkanMemoryManager::createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(vulkanDevice, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(vulkanDevice, stagingBufferMemory);

    VulkanMemoryManager::createImage(static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1, textureMipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, image->image, image->memory);

    VulkanMemoryManager::transitionImageLayout(image->image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, textureMipLevels);
    VulkanMemoryManager::copyBufferToImage(stagingBuffer, image->image, static_cast<uint32_t>(width), static_cast<uint32_t>(height));

    vkDestroyBuffer(vulkanDevice, stagingBuffer, nullptr);
    vkFreeMemory(vulkanDevice, stagingBufferMemory, nullptr);
    VulkanMemoryManager::generateMipmaps(image->image, VK_FORMAT_R8G8B8A8_SRGB, width, height, textureMipLevels);

    image->imageview = VulkanMemoryManager::createImageView(image->image, VK_FORMAT_R8G8B8A8_SRGB, 1, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, textureMipLevels);

    return image;
}

Buffer* VulkanMemoryManager::GetBuffer(uint32_t index)
{
    if (index >= buffers.size())
    {
        throw std::runtime_error("wrong buffer index!");
    }

    return buffers[index];
}

Buffer* VulkanMemoryManager::GetUniformBuffer(UniformBufferIndex index)
{
    if (index >= UniformBufferIndex::UNIFORM_BUFFER_MAX)
    {
        throw std::runtime_error("wrong buffer index!");
    }

    uint32_t innerindex = uniformIndices[index];

    return buffers[innerindex];
}

void VulkanMemoryManager::Init(VkDevice device)
{
    vulkanDevice = device;
    vulkanPhysicalDevice = Application::APP()->GetPhysicalDevice();
    vulkanQueue = Application::APP()->GetGraphicQueue();
    vulkanCommandpool = Application::APP()->GetCommandPool();

    uniformIndices.resize(UNIFORM_BUFFER_MAX);
}

void VulkanMemoryManager::Close()
{
    for (auto buf : buffers)
    {
        buf->close();
        delete buf;
    }
    buffers.clear();
}

void VulkanMemoryManager::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(vulkanDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create vertex buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(vulkanDevice, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(vulkanDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate vertex buffer memory!");
    }

    vkBindBufferMemory(vulkanDevice, buffer, bufferMemory, 0);
}

void VulkanMemoryManager::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}

uint32_t VulkanMemoryManager::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    //should call this function once?
    VkPhysicalDeviceMemoryProperties memProperties = Application::APP()->GetMemProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
    {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type");
}

void VulkanMemoryManager::createImage(uint32_t width, uint32_t height, uint32_t layer, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, 
    VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageCreateFlags flag, VkImage& image, VkDeviceMemory& imageMemory)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = layer;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = numSamples;
    imageInfo.flags = flag;

    if (vkCreateImage(vulkanDevice, &imageInfo, nullptr, &image) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(vulkanDevice, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(vulkanDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(vulkanDevice, image, imageMemory, 0);
}

VkCommandBuffer VulkanMemoryManager::beginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = vulkanCommandpool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(vulkanDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void VulkanMemoryManager::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(vulkanQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(vulkanQueue);

    vkFreeCommandBuffers(vulkanDevice, vulkanCommandpool, 1, &commandBuffer);
}

void VulkanMemoryManager::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layercount, uint32_t mipLevels)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = layercount;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT)
        {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else
    {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0,
        0, nullptr, 0, nullptr, 1, &barrier);

    endSingleTimeCommands(commandBuffer);
}

void VulkanMemoryManager::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = { 0,0,0 };
    region.imageExtent = { width, height, 1 };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endSingleTimeCommands(commandBuffer);
}

VkImageView VulkanMemoryManager::createImageView(VkImage image, VkFormat format, uint32_t layercount, VkImageViewType viewtype, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
{
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = image;
    createInfo.viewType = viewtype;
    createInfo.format = format;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = aspectFlags;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = mipLevels;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = layercount;

    VkImageView result;

    if (vkCreateImageView(vulkanDevice, &createInfo, nullptr, &result) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image views!");
    }

    return result;
}

void VulkanMemoryManager::generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
{
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(vulkanPhysicalDevice, imageFormat, &formatProperties);

    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
    {
        throw std::runtime_error("texture image format does not support linear blitting!");
    }

    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = texWidth;
    int32_t mipHeight = texHeight;

    for (uint32_t i = 1; i < mipLevels; ++i)
    {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        VkImageBlit blit{};
        blit.srcOffsets[0] = { 0,0,0 };
        blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = { 0,0,0 };
        blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    endSingleTimeCommands(commandBuffer);
}

void VulkanMemoryManager::FreeBuffer(VkBuffer buf, VkDeviceMemory devicememory)
{
    vkDestroyBuffer(vulkanDevice, buf, nullptr);
    vkFreeMemory(vulkanDevice, devicememory, nullptr);
}

void VulkanMemoryManager::FreeImage(VkImage image, VkImageView imageview, VkDeviceMemory devicememory)
{
    vkDestroyImageView(vulkanDevice, imageview, nullptr);
    if(image != nullptr) vkDestroyImage(vulkanDevice, image, nullptr);
    vkFreeMemory(vulkanDevice, devicememory, nullptr);
}

void VulkanMemoryManager::MapMemory(VkDeviceMemory devicememory, size_t size, void* data)
{
    void* temp;
    vkMapMemory(vulkanDevice, devicememory, 0, size, 0, &temp);
    memcpy(temp, data, size);
    vkUnmapMemory(vulkanDevice, devicememory);
}

void VulkanMemoryManager::MapMemory(uint32_t index, void* data, size_t size, uint32_t offset)
{
    void* temp;
    uint32_t innerindex = uniformIndices[index];
    VkDeviceSize buffersize = buffers[innerindex]->size;
    if (size != 0) buffersize = size;
    VkDeviceMemory memory = buffers[innerindex]->GetMemory();
    vkMapMemory(vulkanDevice, memory, offset, buffersize, 0, &temp);
    memcpy(temp, data, buffersize);
    vkUnmapMemory(vulkanDevice, memory);
}

Buffer::Buffer() {}

VkDescriptorBufferInfo Buffer::GetDescriptorInfo() const
{
    VkDescriptorBufferInfo result;

    result.buffer = buffer;
    result.offset = 0;
    result.range = offset;

    return result;
}
