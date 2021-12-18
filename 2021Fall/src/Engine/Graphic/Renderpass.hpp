#pragma once

//third party library
#include <vulkan/vulkan.h>

//standard library
#include <vector>

class Renderpass
{
public:
	Renderpass(VkDevice device);

	void init();
	void createRenderPass();
	void createFramebuffers(uint32_t width, uint32_t height, uint32_t layer, uint32_t number = 1);
	void close();

public:
	enum class AttachmentType
	{
		ATTACHMENT_COLOR,
		ATTACHMENT_DEPTH,
		ATTACHMENT_RESOLVE,
	};
	struct Attachment
	{
		AttachmentType type;

		uint32_t bindLocation;
		std::vector<VkImageView> imageViews;
		VkAttachmentDescription attachmentDescription;
	};

	void addAttachment(Attachment attachment);
	VkRenderPass getRenderpass() const;

	void beginRenderpass(VkCommandBuffer commandbuffer, uint32_t index = 0);
	uint32_t getOutputSize() const;

private:
	VkRenderPass renderPassObject;
	std::vector<VkFramebuffer> framebufferObjects;

	std::vector<Attachment> attachments;
	std::vector<VkClearValue> clearValues;

	uint32_t outputSize;

	VkDevice vulkanDevice;

	VkExtent2D extent;
};