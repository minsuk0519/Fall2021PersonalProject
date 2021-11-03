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
	void createFramebuffer();
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
		VkImageView imageView;
		VkAttachmentDescription attachmentDescription;
	};

	void addAttachment(Attachment attachment);
	VkRenderPass getRenderpass() const;

	void beginRenderpass(VkCommandBuffer commandbuffer);
	uint32_t getOutputSize() const;

private:
	VkRenderPass renderPassObject;
	VkFramebuffer framebufferObject;

	std::vector<Attachment> attachments;
	std::vector<VkClearValue> clearValues;

	uint32_t outputSize;

	VkDevice vulkanDevice;
};