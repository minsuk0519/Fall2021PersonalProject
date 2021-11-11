#include "Graphic.hpp"
#include "Engine/Misc/settings.hpp"
#include "Engine/Application.hpp"
#include "VertexInfo.hpp"
#include "Renderpass.hpp"
#include "DescriptorSet.hpp"
#include "Buffer.hpp"
#include "Image.hpp"
#include "Camera.hpp"
#include "Engine/Input/Input.hpp"

//standard library
#include <stdexcept>
#include <unordered_map>
#include <iostream>

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

#define INSTANCE_COUNT 1

Graphic::Graphic(VkDevice device, Application* app) : System(device, app) {}

void Graphic::init()
{
    VulkanMemoryManager::Init(vulkanDevice);

    SetupSwapChain();

    //make quad
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

        uint32_t vertex = VulkanMemoryManager::CreateVertexBuffer(vert.data(), vert.size() * sizeof(PosTexVertex));
        uint32_t index = VulkanMemoryManager::CreateIndexBuffer(indices.data(), indices.size() * sizeof(uint32_t));

        drawtargets.push_back({ {{vertex, index, 6}} });
    }

    //create vertex & index buffer
    {
        std::vector<PosColorTexVertex> vert;
        std::vector<uint32_t> indices;

        std::vector<tinyobj::shape_t> shapes;
        tinyobj::attrib_t attrib;

        //loadModel(attrib, shapes, "data/models/bmw/", "bmw.obj");
        loadModel(attrib, shapes, "data/models/viking_room/", "viking_room.obj");

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
        uint32_t vertex = VulkanMemoryManager::CreateVertexBuffer(vert.data(), vertexbuffermemorysize);

        //index
        size_t indexbuffermemorysize = indices.size() * sizeof(uint32_t);
        uint32_t index = VulkanMemoryManager::CreateIndexBuffer(indices.data(), indexbuffermemorysize);

        VkDeviceSize bufferSize = sizeof(transform) * 3;
        uint32_t uniform = VulkanMemoryManager::CreateUniformBuffer(bufferSize);

        std::vector<glm::vec3> transform_matrices;
        transform_matrices.reserve(INSTANCE_COUNT);
        float scale = 0.4f;
        int midpoint = 30;
        for (int i = 0; i < INSTANCE_COUNT; ++i)
        {
            glm::vec3 vec = glm::vec3(6.0f - scale * (i / midpoint), 0.0f, 3.0f - scale * (i % midpoint));
            vec = glm::vec3(0.0f, 0.0f, -5.0f);
            transform_matrices.push_back(vec);
        }

        size_t instance_size = transform_matrices.size() * sizeof(glm::vec3);
        uint32_t instance = VulkanMemoryManager::CreateVertexBuffer(transform_matrices.data(), instance_size);

        drawtargets.push_back({ {{vertex, index, 11484}}, uniform, instance, INSTANCE_COUNT });
    }

    //create texture image
    {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load("data/models/viking_room/viking_room.png", &texWidth,
            &texHeight, &texChannels, STBI_rgb_alpha);

        if (!pixels)
        {
            throw std::runtime_error("failed to load texture image!");
        }

        images.push_back(VulkanMemoryManager::CreateTextureImage(texWidth, texHeight, pixels));

        stbi_image_free(pixels);
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
        imagesInFlight.resize(swapchainImageSize, VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            if (vkCreateSemaphore(vulkanDevice, &semaphoreInfo, nullptr, &vulkanImageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(vulkanDevice, &semaphoreInfo, nullptr, &vulkanRenderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(vulkanDevice, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create semaphores!");
            }
        }
    }

    camera = new Camera();
    camera->GetTransform().SetPosition(glm::vec3(0.0f, 2.0f, 0.0f));
}

void Graphic::update(float dt)
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
        time += dt * 0.01f;

        transform ubo[3];
        if (Input::isPressed(KeyBinding::KEY_UP)) camera->Move(1.0f * dt, 0.0f);
        if (Input::isPressed(KeyBinding::KEY_DOWN)) camera->Move(-1.0f * dt, 0.0f);
        if (Input::isPressed(KeyBinding::KEY_RIGHT)) camera->Move(0.0f, 1.0f * dt);
        if (Input::isPressed(KeyBinding::KEY_LEFT)) camera->Move(0.0f, -1.0f * dt);
        if(Input::isPressed(KeyBinding::MOUSE_RIGHT)) camera->LookAround(Input::GetMouseMove().x * dt, Input::GetMouseMove().y * dt);

        camera->update(dt);


        ubo[0].objectMat = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(3.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        ubo[0].worldToCamera = camera->GetWorldToCamera();
        ubo[0].cameraToNDC = glm::perspective(glm::radians(45.0f), Settings::GetAspectRatio(), 0.1f, 100.0f);
        ubo[0].cameraToNDC[1][1] *= -1;

        ubo[1].objectMat = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.2f)) * glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        ubo[1].worldToCamera = camera->GetWorldToCamera();
        ubo[1].cameraToNDC = glm::perspective(glm::radians(45.0f), Settings::GetAspectRatio(), 0.1f, 100.0f);
        ubo[1].cameraToNDC[1][1] *= -1;

        ubo[2].objectMat = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        ubo[2].worldToCamera = camera->GetWorldToCamera();
        ubo[2].cameraToNDC = glm::perspective(glm::radians(45.0f), Settings::GetAspectRatio(), 0.1f, 100.0f);
        ubo[2].cameraToNDC[1][1] *= -1;

        VkDeviceMemory mem = VulkanMemoryManager::GetBuffer(drawtargets[1].uniformIndex.value())->GetMemory();
        VulkanMemoryManager::MapMemory(mem, sizeof(transform) * 3, &ubo);
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
    vkDestroySampler(vulkanDevice, vulkanTextureSampler, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(vulkanDevice, vulkanRenderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(vulkanDevice, vulkanImageAvailableSemaphores[i], nullptr);
        vkDestroyFence(vulkanDevice, inFlightFences[i], nullptr);
    }

    for (auto image : images)
    {
        image->close();
        delete image;
    }
    images.clear();

    //for (auto buf : buffers)
    //{
    //    buf->close();
    //    delete buf;
    //}
    //buffers.clear();

    VulkanMemoryManager::Close();

    CloseSwapChain();

    camera->close();
    delete camera;
}

Graphic::~Graphic() {}

void Graphic::SetupSwapChain()
{
    //create swap chain
    {
        vulkanSwapChain = application->CreateSwapChain(swapchainImageSize, vulkanSwapChainImageFormat, vulkanSwapChainExtent);

        VulkanMemoryManager::GetSwapChainImage(vulkanSwapChain, swapchainImageSize, swapchainImages, vulkanSwapChainImageFormat);
    }

    {
        vulkanMSAASamples = getMaxUsableSampleCount();
    }

    //create resources (multisampled color resource)
    {
        framebufferImages.resize(FrameBufferIndex::FRAMEBUFFER_MAX);

        framebufferImages[FrameBufferIndex::COLORATTACHMENT] = VulkanMemoryManager::CreateFrameBufferImage(VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, vulkanSwapChainImageFormat, vulkanMSAASamples);
        framebufferImages[FrameBufferIndex::NORMALATTACHMENT] = VulkanMemoryManager::CreateFrameBufferImage(VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_FORMAT_R16G16B16A16_SFLOAT, vulkanMSAASamples);
        framebufferImages[FrameBufferIndex::POSITIONATTACHMENT] = VulkanMemoryManager::CreateFrameBufferImage(VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_FORMAT_R16G16B16A16_SFLOAT, vulkanMSAASamples);

        framebufferImages[FrameBufferIndex::COLORATTACHMENT_MSAA] = VulkanMemoryManager::CreateFrameBufferImage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, vulkanSwapChainImageFormat, VK_SAMPLE_COUNT_1_BIT);
        framebufferImages[FrameBufferIndex::NORMALATTACHMENT_MSAA] = VulkanMemoryManager::CreateFrameBufferImage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_FORMAT_R16G16B16A16_SFLOAT, VK_SAMPLE_COUNT_1_BIT);
        framebufferImages[FrameBufferIndex::POSITIONATTACHMENT_MSAA] = VulkanMemoryManager::CreateFrameBufferImage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_FORMAT_R16G16B16A16_SFLOAT, VK_SAMPLE_COUNT_1_BIT);

        //depth buffer
        vulkanDepthFormat = findSupportedFormat({
            VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT
            }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

        framebufferImages[FrameBufferIndex::DEPTHATTACHMENT] = VulkanMemoryManager::CreateDepthBuffer(vulkanDepthFormat, vulkanMSAASamples);
    }
}

void Graphic::CloseSwapChain()
{
    descriptorSet->close();
    delete descriptorSet;

    postdescriptorSet->close();
    delete postdescriptorSet;

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

    for (auto image : framebufferImages)
    {
        image->close();
        delete image;
    }
    framebufferImages.clear();

    for (auto image : swapchainImages)
    {
        image->close();
        delete image;
    }
    swapchainImages.clear();

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

void Graphic::DrawDrawtarget(const VkCommandBuffer& cmdBuffer, const DrawTarget& target)
{
    VkBuffer vertexbuffer = VulkanMemoryManager::GetBuffer(target.vertexIndices[0].vertex)->GetBuffer();

    VkBuffer vertexBuffers[] = { vertexbuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers, offsets);

    VkBuffer indexbuffer = VulkanMemoryManager::GetBuffer(target.vertexIndices[0].index)->GetBuffer();
    vkCmdBindIndexBuffer(cmdBuffer, indexbuffer, 0, VK_INDEX_TYPE_UINT32);

    if (target.instancebuffer.has_value())
    {
        VkBuffer instancebuffer = VulkanMemoryManager::GetBuffer(target.instancebuffer.value())->GetBuffer();
        VkBuffer instanceBuffer[] = { instancebuffer };
        vkCmdBindVertexBuffers(cmdBuffer, 1, 1, instanceBuffer, offsets);
    }

    uint32_t instancenumber = target.instancenumber.value_or(1);
    vkCmdDrawIndexed(cmdBuffer, target.vertexIndices[0].indexSize, instancenumber, 0, 0, 0);
}

void Graphic::DefineDrawBehavior()
{
    {
        descriptorSet = new DescriptorSet(vulkanDevice);
        DescriptorSet::Descriptor descriptor;
        descriptor.binding = 0;

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = VulkanMemoryManager::GetBuffer(drawtargets.at(1).uniformIndex.value())->GetBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(transform);

        descriptor.bufferInfo = bufferInfo;
        descriptor.stage = VK_SHADER_STAGE_VERTEX_BIT;
        descriptor.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        descriptorSet->AddDescriptor(descriptor);

        descriptor.binding = 1;
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = images[0]->GetImageView();
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
        imageInfo.sampler = vulkanTextureSampler;
        descriptor.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        descriptor.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

        for (int i = 0; i < COLORATTACHMENT_MAX; ++i)
        {
            descriptor.binding = i;
            imageInfo.imageView = framebufferImages[i + COLORATTACHMENT_MAX]->GetImageView();
            descriptor.imageInfo = imageInfo;
            postdescriptorSet->AddDescriptor(descriptor);
        }

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
        for (int i = 0; i < COLORATTACHMENT_MAX; ++i)
        {
            attach.attachmentDescription.format = framebufferImages[i]->GetFormat();
            attach.bindLocation = i;
            attach.imageViews.push_back(framebufferImages[i]->GetImageView());
            renderpass->addAttachment(attach);
            attach.imageViews.clear();
        }

        attach.type = Renderpass::AttachmentType::ATTACHMENT_RESOLVE;
        attach.attachmentDescription = colorAttachmentResolve;
        for (int i = 0; i < COLORATTACHMENT_MAX; ++i)
        {
            int msaaindex = i + COLORATTACHMENT_MAX;
            attach.attachmentDescription.format = framebufferImages[msaaindex]->GetFormat();
            attach.bindLocation = msaaindex;
            attach.imageViews.push_back(framebufferImages[msaaindex]->GetImageView());
            renderpass->addAttachment(attach);
            attach.imageViews.clear();
        }

        attach.attachmentDescription = depthAttachment;
        attach.type = Renderpass::AttachmentType::ATTACHMENT_DEPTH;
        attach.bindLocation = 6;
        attach.imageViews.push_back(framebufferImages[6]->GetImageView());
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
        for (auto swapimage : swapchainImages)
        {
            attach.imageViews.push_back(swapimage->GetImageView());
        }
        postrenderpass->addAttachment(attach);

        postrenderpass->createRenderPass();
        postrenderpass->createFramebuffers(static_cast<uint32_t>(swapchainImageSize));
    }

    //create graphic pipeline
    {
        graphicPipeline = new GraphicPipeline(vulkanDevice);

        graphicPipeline->AddShaderStages("data/shaders/baserendervert.spv", VK_SHADER_STAGE_VERTEX_BIT);
        graphicPipeline->AddShaderStages("data/shaders/baserenderfrag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

        std::array<VkVertexInputBindingDescription, 2> bindingDescriptions;
        bindingDescriptions[0] = PosColorTexVertex::getBindingDescription();
        bindingDescriptions[1].binding = 1;
        bindingDescriptions[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
        bindingDescriptions[1].stride = sizeof(glm::vec3);

        auto vertdesc = PosColorTexVertex::getAttributeDescriptions();
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions;
        attributeDescriptions[0] = vertdesc[0];
        attributeDescriptions[1] = vertdesc[1];
        attributeDescriptions[2] = vertdesc[2];
        attributeDescriptions[3].binding = 1;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[3].offset = 0;

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
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
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = application->GetCommandPool();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(vulkanDevice, &allocInfo, &vulkanCommandBuffers) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }

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

            descriptorSet->BindDescriptorSet(vulkanCommandBuffers, graphicPipeline->GetPipelinLayout(), 0);
            DrawDrawtarget(vulkanCommandBuffers, drawtargets[1]);

            descriptorSet->BindDescriptorSet(vulkanCommandBuffers, graphicPipeline->GetPipelinLayout(), 1);
            DrawDrawtarget(vulkanCommandBuffers, drawtargets[1]);

            descriptorSet->BindDescriptorSet(vulkanCommandBuffers, graphicPipeline->GetPipelinLayout(), 2);
            DrawDrawtarget(vulkanCommandBuffers, drawtargets[1]);

            vkCmdEndRenderPass(vulkanCommandBuffers);

            if (vkEndCommandBuffer(vulkanCommandBuffers) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to recored command buffer!");
            }
        }
    }

    {
        vulkanpostCommandBuffer.resize(swapchainImageSize);

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

            postdescriptorSet->BindDescriptorSet(vulkanpostCommandBuffer[i], postgraphicPipeline->GetPipelinLayout(), 0);

            DrawDrawtarget(vulkanpostCommandBuffer[i], drawtargets[0]);

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

void Graphic::loadModel(tinyobj::attrib_t& attrib, std::vector<tinyobj::shape_t>& shapes, const std::string& path, const std::string& filename)
{
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, (path + filename).c_str(), path.c_str()))
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

void DrawTarget::AddVertex(VertexInfo info)
{
    vertexIndices.push_back(info);
}
