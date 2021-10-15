#include "Graphic.hpp"
#include "Engine/settings.hpp"
#include "Engine/Application.hpp"

//standard library
#include <stdexcept>

//3rd party library
#include <vulkan/vulkan.h>

Graphic::Graphic(VkDevice device, Application* app) : System(device, app) {}

void Graphic::init()
{
    //create renderpass
    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = application->GetSwapChainImageFormat();
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

        if (vkCreateRenderPass(vulkanDevice, &renderPassInfo, nullptr, &vulkanRenderPass) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    graphicPipeline = new GraphicPipeline(vulkanDevice);
    graphicPipeline->init(vulkanRenderPass);

    auto swapchainImages = application->GetSwapChainImageViews();
    vulkanSwapChainFramebuffers.resize(swapchainImages.size());

    for (size_t i = 0; i < swapchainImages.size(); ++i)
    {
        VkImageView attachments[] = {
            swapchainImages[i]
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

    //create buffer
    {
        commandBuffers.resize(vulkanSwapChainFramebuffers.size());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = application->GetCommandPool();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

        if (vkAllocateCommandBuffers(vulkanDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }

        for (size_t i = 0; i < commandBuffers.size(); ++i)
        {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = 0;
            beginInfo.pInheritanceInfo = nullptr;

            if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
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

            vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                graphicPipeline->GetPipeline());

            vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

            vkCmdEndRenderPass(commandBuffers[i]);

            if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to recored command buffer!");
            }
        }
    }
}

void Graphic::update(float dt)
{
}

void Graphic::close()
{
    for (auto frameBuffer : vulkanSwapChainFramebuffers)
    {
        vkDestroyFramebuffer(vulkanDevice, frameBuffer, nullptr);
    }

	graphicPipeline->close();
	delete graphicPipeline;

    vkDestroyRenderPass(vulkanDevice, vulkanRenderPass, nullptr);
}