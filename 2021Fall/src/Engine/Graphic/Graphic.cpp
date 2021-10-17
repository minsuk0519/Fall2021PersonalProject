#include "Graphic.hpp"
#include "Engine/settings.hpp"
#include "Engine/Application.hpp"

//standard library
#include <stdexcept>

//3rd party library
#include <vulkan/vulkan.h>

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

Graphic::Graphic(VkDevice device, Application* app) : System(device, app) {}

void Graphic::init()
{
    SetupSwapChain();

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

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { vulkanImageAvailableSemaphores[currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vulkanCommandBuffers[imageIndex];

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
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(vulkanDevice, vulkanRenderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(vulkanDevice, vulkanImageAvailableSemaphores[i], nullptr);
        vkDestroyFence(vulkanDevice, inFlightFences[i], nullptr);
    }

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

    //image views
    {
        vulkanSwapChainImageViews.resize(vulkanSwapChainImages.size());

        for (size_t i = 0; i < vulkanSwapChainImages.size(); i++)
        {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = vulkanSwapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = vulkanSwapChainImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(vulkanDevice, &createInfo, nullptr, &vulkanSwapChainImageViews[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create image views!");
            }
        }
    }

    //create renderpass
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

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(vulkanDevice, &renderPassInfo, nullptr, &vulkanRenderPass) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    //create graphic pipeline
    {
        graphicPipeline = new GraphicPipeline(vulkanDevice);
        graphicPipeline->init(vulkanRenderPass);
    }
    
    //create framebuffer
    {
        vulkanSwapChainFramebuffers.resize(vulkanSwapChainImageViews.size());

        for (size_t i = 0; i < vulkanSwapChainImageViews.size(); ++i)
        {
            VkImageView attachments[] = {
                vulkanSwapChainImageViews[i]
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = vulkanRenderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = Settings::windowWidth;
            framebufferInfo.height = Settings::windowHeight;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(vulkanDevice, &framebufferInfo, nullptr, &vulkanSwapChainFramebuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    //create command buffer
    {
        vulkanCommandBuffers.resize(vulkanSwapChainFramebuffers.size());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = application->GetCommandPool();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)vulkanCommandBuffers.size();

        if (vkAllocateCommandBuffers(vulkanDevice, &allocInfo, vulkanCommandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }

        for (size_t i = 0; i < vulkanCommandBuffers.size(); ++i)
        {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = 0;
            beginInfo.pInheritanceInfo = nullptr;

            if (vkBeginCommandBuffer(vulkanCommandBuffers[i], &beginInfo) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to begin recording command buffer!");
            }

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = vulkanRenderPass;
            renderPassInfo.framebuffer = vulkanSwapChainFramebuffers[i];

            renderPassInfo.renderArea.offset = { 0,0 };
            renderPassInfo.renderArea.extent = { Settings::windowWidth, Settings::windowHeight };

            VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearColor;

            vkCmdBeginRenderPass(vulkanCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(vulkanCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                graphicPipeline->GetPipeline());

            vkCmdDraw(vulkanCommandBuffers[i], 3, 1, 0, 0);

            vkCmdEndRenderPass(vulkanCommandBuffers[i]);

            if (vkEndCommandBuffer(vulkanCommandBuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to recored command buffer!");
            }
        }
    }
}

void Graphic::CloseSwapChain()
{
    for (auto frameBuffer : vulkanSwapChainFramebuffers)
    {
        vkDestroyFramebuffer(vulkanDevice, frameBuffer, nullptr);
    }

    vkFreeCommandBuffers(vulkanDevice, application->GetCommandPool(), static_cast<uint32_t>(vulkanCommandBuffers.size()), vulkanCommandBuffers.data());

    graphicPipeline->close();
    delete graphicPipeline;

    vkDestroyRenderPass(vulkanDevice, vulkanRenderPass, nullptr);

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
}
