#include "Renderpass.hpp"
#include "Engine/Misc/settings.hpp"

//standard library
#include <stdexcept>

Renderpass::Renderpass(VkDevice device) : vulkanDevice(device) {}

void Renderpass::init()
{
   
}

void Renderpass::createRenderPass()
{
    std::vector<VkAttachmentReference> colorattachmentRefs;
    std::vector<VkAttachmentReference> resolvedattachmentRefs;
    VkAttachmentReference depthAttachmentRef;
    std::vector<VkAttachmentDescription> attachmentdescription;
    bool nodepth = true;
    
    for (const auto& attach : attachments)
    {
        attachmentdescription.push_back(attach.attachmentDescription);

        VkAttachmentReference attachmentRef;

        auto findpred = [&](const VkAttachmentReference& target)->bool {
            return target.attachment == attach.bindLocation;
        };

        if (attach.type == AttachmentType::ATTACHMENT_COLOR)
        {
            if (std::find_if(colorattachmentRefs.begin(), colorattachmentRefs.end(), findpred) != colorattachmentRefs.end()) continue;

            attachmentRef.attachment = attach.bindLocation;
            attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            colorattachmentRefs.push_back(attachmentRef);
        }
        else if (attach.type == AttachmentType::ATTACHMENT_RESOLVE)
        {
            if (std::find_if(resolvedattachmentRefs.begin(), resolvedattachmentRefs.end(), findpred) != resolvedattachmentRefs.end()) continue;
            
            attachmentRef.attachment = attach.bindLocation;
            attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            resolvedattachmentRefs.push_back(attachmentRef);
        }
        else
        {
            attachmentRef.attachment = attach.bindLocation;
            attachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depthAttachmentRef = attachmentRef;
            nodepth = false;
        }
    }

    //if (colorattachmentRefs.size() != resolvedattachmentRefs.size())
    //{
    //    throw std::runtime_error("Error : renderpass color attachment size != resolved attachment size");
    //}

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t>(colorattachmentRefs.size());
    subpass.pColorAttachments = colorattachmentRefs.data();
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    if (nodepth) subpass.pDepthStencilAttachment = nullptr;
    subpass.pResolveAttachments = resolvedattachmentRefs.data();

    outputSize = static_cast<uint32_t>(colorattachmentRefs.size());

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentdescription.size());
    renderPassInfo.pAttachments = attachmentdescription.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(vulkanDevice, &renderPassInfo, nullptr, &renderPassObject) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }
}

void Renderpass::createFramebuffers(uint32_t width, uint32_t height, uint32_t layer, uint32_t number)
{
    extent = { width, height };

    framebufferObjects.resize(number);

    for (uint32_t j = 0; j < number; ++j)
    {
        std::vector<VkImageView> imageAttachments;
        uint32_t attachmentsize = static_cast<uint32_t>(attachments.size());
        imageAttachments.resize(attachmentsize);
        clearValues.resize(attachmentsize);

        for (uint32_t i = 0; i < attachmentsize; ++i)
        {
            imageAttachments[i] = attachments[i].imageViews[j];

            if (attachments[i].type == AttachmentType::ATTACHMENT_DEPTH) clearValues[i].depthStencil = { 1.0f, 0 };
            else clearValues[i].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        }

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPassObject;
        framebufferInfo.attachmentCount = attachmentsize;
        framebufferInfo.pAttachments = imageAttachments.data();
        framebufferInfo.width = width;
        framebufferInfo.height = height;
        framebufferInfo.layers = layer;

        if (vkCreateFramebuffer(vulkanDevice, &framebufferInfo, nullptr, &framebufferObjects[j]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void Renderpass::close()
{
    for (auto& framebuffer : framebufferObjects)
    {
        vkDestroyFramebuffer(vulkanDevice, framebuffer, nullptr);
    }

    vkDestroyRenderPass(vulkanDevice, renderPassObject, nullptr);
}

void Renderpass::addAttachment(Attachment attachment)
{
    attachments.push_back(attachment);
}

VkRenderPass Renderpass::getRenderpass() const
{
    return renderPassObject;
}

void Renderpass::beginRenderpass(VkCommandBuffer commandbuffer, uint32_t index)
{
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPassObject;
    renderPassInfo.framebuffer = framebufferObjects[index];

    renderPassInfo.renderArea.offset = { 0,0 };
    renderPassInfo.renderArea.extent = extent;

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandbuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

uint32_t Renderpass::getOutputSize() const
{
    return outputSize;
}
