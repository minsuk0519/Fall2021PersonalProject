#include "Graphic.hpp"
#include "Engine/Misc/settings.hpp"
#include "Engine/Application.hpp"
#include "VertexInfo.hpp"
#include "Renderpass.hpp"
#include "DescriptorSet.hpp"
#include "Buffer.hpp"

//standard library
#include <stdexcept>
#include <unordered_map>

//3rd party library
#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

Graphic::Graphic(VkDevice device, Application* app) : System(device, app) {}

void Graphic::init()
{
    VulkanMemoryManager::Init(vulkanDevice);

    SetupSwapChain();

    //create vertex & index buffer
    {
        std::vector<PosColorTexVertex> vert;
        std::vector<uint32_t> indices;

        std::vector<tinyobj::shape_t> shapes;
        tinyobj::attrib_t attrib;

        loadModel(attrib, shapes, "data/models/viking_room/viking_room.obj");

        std::unordered_map<PosColorTexVertex, uint32_t> uniqueVertices{};

        for (const auto& shape : shapes)
        {
            for (const auto& index : shape.mesh.indices)
            {
                PosColorTexVertex vertex{};

                vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };

                vertex.color = { 1.0f, 1.0f, 1.0f };

                //remove duplicate vertices
                if (uniqueVertices.count(vertex) == 0)
                {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vert.size());
                    vert.push_back(vertex);
                }

                indices.push_back(uniqueVertices[vertex]);
            }
        }

        //vertex
        size_t vertexbuffermemorysize = vert.size() * sizeof(PosColorTexVertex);
        buffers.push_back(VulkanMemoryManager::CreateVertexBuffer(vert.data(), vertexbuffermemorysize));

        //index
        size_t indexbuffermemorysize = indices.size() * sizeof(uint32_t);
        buffers.push_back(VulkanMemoryManager::CreateIndexBuffer(indices.data(), indexbuffermemorysize));

        VkDeviceSize bufferSize = sizeof(transform);
        buffers.push_back(VulkanMemoryManager::CreateUniformBuffer(bufferSize));
    }

    {
        std::vector<PosTexVertex> vert = {
            {{-1.0f, -1.0f}, {0.0f, 0.0f}},
            {{-1.0f,  1.0f}, {0.0f, 1.0f}},
            {{ 1.0f,  1.0f}, {1.0f, 1.0f}},
            {{ 1.0f, -1.0f}, {1.0f, 0.0f}},
        };

        std::vector<uint32_t> indices = {
            0, 1, 2, 0, 2, 3,
        };

        buffers.push_back(VulkanMemoryManager::CreateVertexBuffer(vert.data(), vert.size() * sizeof(PosTexVertex)));
        buffers.push_back(VulkanMemoryManager::CreateIndexBuffer(indices.data(), indices.size() * sizeof(uint32_t)));
    }

    //create texture image
    {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load("data/models/viking_room/viking_room.png", &texWidth,
            &texHeight, &texChannels, STBI_rgb_alpha);

        VkDeviceSize imageSize = texWidth * texHeight * 4;
        textureMipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

        if (!pixels)
        {
            throw std::runtime_error("failed to load texture image!");
        }

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        VulkanMemoryManager::createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(vulkanDevice, stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(vulkanDevice, stagingBufferMemory);

        stbi_image_free(pixels);

        VulkanMemoryManager::createImage(static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), textureMipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkanTextureImage, vulkanTextureImageMemory);

        VulkanMemoryManager::transitionImageLayout(vulkanTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, textureMipLevels);
        VulkanMemoryManager::copyBufferToImage(stagingBuffer, vulkanTextureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

        vkDestroyBuffer(vulkanDevice, stagingBuffer, nullptr);
        vkFreeMemory(vulkanDevice, stagingBufferMemory, nullptr);
        VulkanMemoryManager::generateMipmaps(vulkanTextureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, textureMipLevels);

        vulkanTextureImageView = VulkanMemoryManager::createImageView(vulkanTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, textureMipLevels);
    }

    //sampler
    {
        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;

        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;

        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = static_cast<float>(textureMipLevels / 2);
        samplerInfo.maxLod = static_cast<float>(textureMipLevels);

        if (vkCreateSampler(vulkanDevice, &samplerInfo, nullptr, &vulkanTextureSampler))
        {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }

    DefineDrawBehavior();

    //create semaphore
    {
        vulkanImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        vulkanRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        imagesInFlight.resize(vulkanSwapChainImages.size(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            if (vkCreateSemaphore(vulkanDevice, &semaphoreInfo, nullptr,
                    &vulkanImageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(vulkanDevice, &semaphoreInfo, nullptr,
                    &vulkanRenderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(vulkanDevice, &fenceInfo, nullptr,
                    &inFlightFences[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create semaphores!");
            }
        }
    }
}

void Graphic::update(float /*dt*/)
{
    vkWaitForFences(vulkanDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(vulkanDevice, vulkanSwapChain,
        UINT64_MAX, vulkanImageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
    {
        vkWaitForFences(vulkanDevice, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    imagesInFlight[imageIndex] = inFlightFences[currentFrame];

    //update uniform buffer
    {
        static float time = 0; 
        time += 0.0001f;

        transform ubo{};
        ubo.objectMat = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.8f));
        ubo.worldToCamera = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.cameraToNDC = glm::perspective(glm::radians(45.0f), Settings::GetAspectRatio(), 0.1f, 10.0f);
        ubo.cameraToNDC[1][1] *= -1;

        VulkanMemoryManager::MapMemory(buffers[2]->GetMemory(), sizeof(transform), &ubo);
    }

    //pre render
    {
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        std::array<VkCommandBuffer, 1> bufferlist = { vulkanCommandBuffers };

        submitInfo.commandBufferCount = static_cast<uint32_t>(bufferlist.size());
        submitInfo.pCommandBuffers = bufferlist.data();

        if (vkQueueSubmit(application->GetGraphicQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to submit draw command buffer!");
        }
        vkQueueWaitIdle(application->GetGraphicQueue());
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { vulkanImageAvailableSemaphores[currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    std::array<VkCommandBuffer, 1> bufferlist = { vulkanpostCommandBuffer[imageIndex] };

    submitInfo.commandBufferCount = static_cast<uint32_t>(bufferlist.size());
    submitInfo.pCommandBuffers = bufferlist.data();

    VkSemaphore signalSemaphores[] = { vulkanRenderFinishedSemaphores[currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(vulkanDevice, 1, &inFlightFences[currentFrame]);

    if (vkQueueSubmit(application->GetGraphicQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { vulkanSwapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    result = vkQueuePresentKHR(application->GetPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || application->framebufferSizeUpdate)
    {
        application->framebufferSizeUpdate = false;
        RecreateSwapChain();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }
     
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Graphic::close()
{
    for (auto& deviceMem : vulkanDeviceMemories)
    {
        vkFreeMemory(vulkanDevice, deviceMem, nullptr);
    }

    vkDestroySampler(vulkanDevice, vulkanTextureSampler, nullptr);
    vkDestroyImageView(vulkanDevice, vulkanTextureImageView, nullptr);

    vkDestroyImage(vulkanDevice, vulkanTextureImage, nullptr);
    vkFreeMemory(vulkanDevice, vulkanTextureImageMemory, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(vulkanDevice, vulkanRenderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(vulkanDevice, vulkanImageAvailableSemaphores[i], nullptr);
        vkDestroyFence(vulkanDevice, inFlightFences[i], nullptr);
    }

    for (auto buf : buffers)
    {
        buf->close();
        delete buf;
    }
    buffers.clear();

    CloseSwapChain();
}

Graphic::~Graphic() {}

void Graphic::SetupSwapChain()
{
    //create swap chain
    {
        uint32_t imageCount;
        vulkanSwapChain = application->CreateSwapChain(imageCount, vulkanSwapChainImageFormat, vulkanSwapChainExtent);

        vkGetSwapchainImagesKHR(vulkanDevice, vulkanSwapChain, &imageCount, nullptr);
        vulkanSwapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(vulkanDevice, vulkanSwapChain, &imageCount, vulkanSwapChainImages.data());
    }

    {
        vulkanMSAASamples = getMaxUsableSampleCount();
    }

    //image views
    {
        vulkanSwapChainImageViews.resize(vulkanSwapChainImages.size());

        for (size_t i = 0; i < vulkanSwapChainImages.size(); i++)
        {
            vulkanSwapChainImageViews[i] = VulkanMemoryManager::createImageView(vulkanSwapChainImages[i], vulkanSwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
        }
    }

    //create resources (multisampled color resource)
    {
        VkFormat colorFormat = vulkanSwapChainImageFormat;

        for (int i = 0; i < RENDERPASS::COLORATTACHMENT_MAX; ++i)
        {
            VulkanMemoryManager::createImage(Settings::windowWidth, Settings::windowHeight, 1, vulkanMSAASamples, colorFormat,
                VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkanColorImage[i], vulkanColorImageMemory[i]);

            vulkanColorImageView[i] = VulkanMemoryManager::createImageView(vulkanColorImage[i], colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);

            int indexMSAA = i + RENDERPASS::COLORATTACHMENT_MAX;
            VkFormat formatMSAA = colorFormat;

            if (i == RENDERPASS::NORMALATTACHMENT_MSAA) formatMSAA = VK_FORMAT_R16G16B16A16_SFLOAT;

            //color image should be r8g8b8a8_srgb : same as vulkanswapchainimageformat
            VulkanMemoryManager::createImage(Settings::windowWidth, Settings::windowHeight, 1, VK_SAMPLE_COUNT_1_BIT, formatMSAA,
                VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkanColorImage[indexMSAA], vulkanColorImageMemory[indexMSAA]);

            vulkanColorImageView[indexMSAA] = VulkanMemoryManager::createImageView(vulkanColorImage[indexMSAA], formatMSAA, VK_IMAGE_ASPECT_COLOR_BIT, 1);
        }
    }

    //depth buffer
    {
        vulkanDepthFormat = findSupportedFormat({
            VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT
            }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

        VulkanMemoryManager::createImage(Settings::windowWidth, Settings::windowHeight, 1, vulkanMSAASamples, vulkanDepthFormat, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkanDepthImage, vulkanDepthImageMemory);

        vulkanDepthImageView = VulkanMemoryManager::createImageView(vulkanDepthImage, vulkanDepthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

        VulkanMemoryManager::transitionImageLayout(vulkanDepthImage, vulkanDepthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
    }
}

void Graphic::CloseSwapChain()
{
    descriptorSet->close();
    delete descriptorSet;

    postdescriptorSet->close();
    delete postdescriptorSet;

    for (int i = 0; i < RENDERPASS::COLORATTACHMENT_MAX; ++i)
    {
        vkDestroyImageView(vulkanDevice, vulkanColorImageView[i], nullptr);
        vkDestroyImage(vulkanDevice, vulkanColorImage[i], nullptr);
        vkFreeMemory(vulkanDevice, vulkanColorImageMemory[i], nullptr);

        vkDestroyImageView(vulkanDevice, vulkanColorImageView[i + RENDERPASS::COLORATTACHMENT_MAX], nullptr);
        vkDestroyImage(vulkanDevice, vulkanColorImage[i + RENDERPASS::COLORATTACHMENT_MAX], nullptr);
        vkFreeMemory(vulkanDevice, vulkanColorImageMemory[i + RENDERPASS::COLORATTACHMENT_MAX], nullptr);
    }

    vkDestroyImageView(vulkanDevice, vulkanDepthImageView, nullptr);
    vkDestroyImage(vulkanDevice, vulkanDepthImage, nullptr);
    vkFreeMemory(vulkanDevice, vulkanDepthImageMemory, nullptr);

    vkFreeCommandBuffers(vulkanDevice, application->GetCommandPool(), 1, &vulkanCommandBuffers);
    vkFreeCommandBuffers(vulkanDevice, application->GetCommandPool(), static_cast<uint32_t>(vulkanpostCommandBuffer.size()), vulkanpostCommandBuffer.data());

    renderpass->close();
    delete renderpass;

    postrenderpass->close();
    delete postrenderpass;

    graphicPipeline->close();
    delete graphicPipeline;

    postgraphicPipeline->close();
    delete postgraphicPipeline;

    for (auto imageView : vulkanSwapChainImageViews)
    {
        vkDestroyImageView(vulkanDevice, imageView, nullptr);
    }

    vkDestroySwapchainKHR(vulkanDevice, vulkanSwapChain, nullptr);
}

void Graphic::RecreateSwapChain()
{
    while (Settings::windowWidth == 0 || Settings::windowHeight == 0)
    {
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(vulkanDevice);

    CloseSwapChain();
    SetupSwapChain();
    DefineDrawBehavior();
}

void Graphic::DefineDrawBehavior()
{
    {
        descriptorSet = new DescriptorSet(vulkanDevice);
        DescriptorSet::Descriptor descriptor;
        descriptor.binding = 0;

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = buffers[2]->GetBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(transform);

        descriptor.bufferInfo = bufferInfo;
        descriptor.stage = VK_SHADER_STAGE_VERTEX_BIT;
        descriptor.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorSet->AddDescriptor(descriptor);

        descriptor.binding = 1;
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = vulkanTextureImageView;
        imageInfo.sampler = vulkanTextureSampler;

        descriptor.imageInfo = imageInfo;
        descriptor.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        descriptor.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorSet->AddDescriptor(descriptor);

        descriptorSet->CreateDescriptorSet();
    }

    {
        postdescriptorSet = new DescriptorSet(vulkanDevice);
        DescriptorSet::Descriptor descriptor;
        descriptor.binding = 0;
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = vulkanColorImageView[RENDERPASS::COLORATTACHMENT_MSAA];
        imageInfo.sampler = vulkanTextureSampler;
        descriptor.imageInfo = imageInfo;
        descriptor.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        descriptor.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        postdescriptorSet->AddDescriptor(descriptor);

        descriptor.binding = 1;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = vulkanColorImageView[RENDERPASS::NORMALATTACHMENT_MSAA];
        imageInfo.sampler = vulkanTextureSampler;
        descriptor.imageInfo = imageInfo;
        descriptor.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        descriptor.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        postdescriptorSet->AddDescriptor(descriptor);

        descriptor.binding = 2;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = vulkanColorImageView[RENDERPASS::POSITIONATTACHMENT_MSAA];
        imageInfo.sampler = vulkanTextureSampler;
        descriptor.imageInfo = imageInfo;
        descriptor.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        descriptor.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        postdescriptorSet->AddDescriptor(descriptor);

        postdescriptorSet->CreateDescriptorSet();
    }

    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = vulkanSwapChainImageFormat;
        colorAttachment.samples = vulkanMSAASamples;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription colorAttachmentResolve{};
        colorAttachmentResolve.format = vulkanSwapChainImageFormat;
        colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        vulkanDepthFormat = findSupportedFormat({
    VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT
            }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = vulkanDepthFormat;
        depthAttachment.samples = vulkanMSAASamples;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        renderpass = new Renderpass(vulkanDevice);
        Renderpass::Attachment attach;
        attach.type = Renderpass::AttachmentType::ATTACHMENT_COLOR;
        attach.attachmentDescription = colorAttachment;
        attach.bindLocation = 0;
        attach.imageViews.push_back(vulkanColorImageView[0]);
        renderpass->addAttachment(attach);
        attach.imageViews.clear();

        attach.bindLocation = 1;
        attach.imageViews.push_back(vulkanColorImageView[1]);
        renderpass->addAttachment(attach);
        attach.imageViews.clear();

        attach.bindLocation = 2;
        attach.imageViews.push_back(vulkanColorImageView[2]);
        renderpass->addAttachment(attach);
        attach.imageViews.clear();

        attach.attachmentDescription = colorAttachmentResolve;
        attach.type = Renderpass::AttachmentType::ATTACHMENT_RESOLVE;
        attach.bindLocation = 3;
        attach.imageViews.push_back(vulkanColorImageView[3]);
        renderpass->addAttachment(attach);
        attach.imageViews.clear();

        attach.bindLocation = 4;
        attach.imageViews.push_back(vulkanColorImageView[4]);
        renderpass->addAttachment(attach);
        attach.imageViews.clear();

        attach.bindLocation = 5;
        attach.imageViews.push_back(vulkanColorImageView[5]);
        renderpass->addAttachment(attach);
        attach.imageViews.clear();

        attach.attachmentDescription = depthAttachment;
        attach.type = Renderpass::AttachmentType::ATTACHMENT_DEPTH;
        attach.bindLocation = 6;
        attach.imageViews.push_back(vulkanDepthImageView);
        renderpass->addAttachment(attach);
        attach.imageViews.clear();

        renderpass->createRenderPass();
        renderpass->createFramebuffers();
    }

    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = vulkanSwapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        postrenderpass = new Renderpass(vulkanDevice);
        Renderpass::Attachment attach;
        attach.type = Renderpass::AttachmentType::ATTACHMENT_COLOR;
        attach.attachmentDescription = colorAttachment;
        attach.bindLocation = 0;
        for (auto& swapimageview : vulkanSwapChainImageViews)
        {
            attach.imageViews.push_back(swapimageview);
        }
        postrenderpass->addAttachment(attach);

        postrenderpass->createRenderPass();
        postrenderpass->createFramebuffers(static_cast<uint32_t>(vulkanSwapChainImageViews.size()));
    }

    //create graphic pipeline
    {
        graphicPipeline = new GraphicPipeline(vulkanDevice);

        graphicPipeline->AddShaderStages("data/shaders/baserendervert.spv", VK_SHADER_STAGE_VERTEX_BIT);
        graphicPipeline->AddShaderStages("data/shaders/baserenderfrag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

        VkVertexInputBindingDescription bindingDescription = PosColorTexVertex::getBindingDescription();
        auto attributeDescriptions = PosColorTexVertex::getAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        graphicPipeline->init(renderpass->getRenderpass(), descriptorSet->GetSetLayout(), vulkanMSAASamples, vertexInputInfo, 3);
    }

    {
        postgraphicPipeline = new GraphicPipeline(vulkanDevice);

        postgraphicPipeline->AddShaderStages("data/shaders/deferredvert.spv", VK_SHADER_STAGE_VERTEX_BIT);
        postgraphicPipeline->AddShaderStages("data/shaders/deferredfrag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

        VkVertexInputBindingDescription bindingDescription = PosTexVertex::getBindingDescription();
        auto attributeDescriptions = PosTexVertex::getAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        postgraphicPipeline->init(postrenderpass->getRenderpass(), postdescriptorSet->GetSetLayout(), VK_SAMPLE_COUNT_1_BIT, vertexInputInfo, 1);
    }

    //create command buffer
    {
        //vulkanCommandBuffers.resize(vulkanSwapChainImages.size());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = application->GetCommandPool();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;// (uint32_t)vulkanCommandBuffers.size();

        if (vkAllocateCommandBuffers(vulkanDevice, &allocInfo, &vulkanCommandBuffers/*vulkanCommandBuffers.data()*/) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }

        //for (size_t i = 0; i < vulkanCommandBuffers.size(); ++i)
        {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = 0;
            beginInfo.pInheritanceInfo = nullptr;

            if (vkBeginCommandBuffer(vulkanCommandBuffers, &beginInfo) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to begin recording command buffer!");
            }

            renderpass->beginRenderpass(vulkanCommandBuffers);

            vkCmdBindPipeline(vulkanCommandBuffers, VK_PIPELINE_BIND_POINT_GRAPHICS,
                graphicPipeline->GetPipeline());

            VkBuffer vertexBuffers[] = { buffers[0]->GetBuffer() };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(vulkanCommandBuffers, 0, 1, vertexBuffers, offsets);

            //use 32 byte uint for using more than 65535 byte indices
            vkCmdBindIndexBuffer(vulkanCommandBuffers, buffers[1]->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

            descriptorSet->BindDescriptorSet(vulkanCommandBuffers, graphicPipeline->GetPipelinLayout());

            vkCmdDrawIndexed(vulkanCommandBuffers, 11484, 1, 0, 0, 0);

            vkCmdEndRenderPass(vulkanCommandBuffers);

            if (vkEndCommandBuffer(vulkanCommandBuffers) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to recored command buffer!");
            }
        }
    }

    {
        vulkanpostCommandBuffer.resize(vulkanSwapChainImages.size());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = application->GetCommandPool();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)vulkanpostCommandBuffer.size();

        if (vkAllocateCommandBuffers(vulkanDevice, &allocInfo, vulkanpostCommandBuffer.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }

        for (size_t i = 0; i < vulkanpostCommandBuffer.size(); ++i)
        {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = 0;
            beginInfo.pInheritanceInfo = nullptr;

            if (vkBeginCommandBuffer(vulkanpostCommandBuffer[i], &beginInfo) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to begin recording command buffer!");
            }

            postrenderpass->beginRenderpass(vulkanpostCommandBuffer[i], static_cast<uint32_t>(i));

            vkCmdBindPipeline(vulkanpostCommandBuffer[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                postgraphicPipeline->GetPipeline());

            VkBuffer vertexBuffers[] = { buffers[3]->GetBuffer() };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(vulkanpostCommandBuffer[i], 0, 1, vertexBuffers, offsets);

            //use 32 byte uint for using more than 65535 byte indices
            vkCmdBindIndexBuffer(vulkanpostCommandBuffer[i], buffers[4]->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

            postdescriptorSet->BindDescriptorSet(vulkanpostCommandBuffer[i], postgraphicPipeline->GetPipelinLayout());

            vkCmdDrawIndexed(vulkanpostCommandBuffer[i], 6, 1, 0, 0, 0);

            vkCmdEndRenderPass(vulkanpostCommandBuffer[i]);

            if (vkEndCommandBuffer(vulkanpostCommandBuffer[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to recored command buffer!");
            }
        }
    }
}

VkFormat Graphic::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(application->GetPhysicalDevice(), format, &properties);

        if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features)
        {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported forma!");
}

void Graphic::loadModel(tinyobj::attrib_t& attrib, std::vector<tinyobj::shape_t>& shapes, const char* path)
{
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path))
    {
        throw std::runtime_error("failed to load obj file : " + warn + err);
    }
}

VkSampleCountFlagBits Graphic::getMaxUsableSampleCount()
{
    VkPhysicalDeviceProperties physicalDeviceProperties = application->GetDeviceProperties();

    VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts &
        physicalDeviceProperties.limits.framebufferDepthSampleCounts;

    if (counts & VK_SAMPLE_COUNT_64_BIT) return VK_SAMPLE_COUNT_64_BIT;
    if (counts & VK_SAMPLE_COUNT_32_BIT) return VK_SAMPLE_COUNT_32_BIT;
    if (counts & VK_SAMPLE_COUNT_16_BIT) return VK_SAMPLE_COUNT_16_BIT;
    if (counts & VK_SAMPLE_COUNT_8_BIT) return VK_SAMPLE_COUNT_8_BIT;
    if (counts & VK_SAMPLE_COUNT_4_BIT) return VK_SAMPLE_COUNT_4_BIT;
    if (counts & VK_SAMPLE_COUNT_2_BIT) return VK_SAMPLE_COUNT_2_BIT;

    return VK_SAMPLE_COUNT_1_BIT;
}
