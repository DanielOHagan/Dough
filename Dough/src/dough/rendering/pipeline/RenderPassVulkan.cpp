#include "dough/rendering/pipeline/RenderPassVulkan.h"

namespace DOH {

	RenderPassVulkan::RenderPassVulkan()
	:	mRenderPass(VK_NULL_HANDLE) 
	{
	}

	RenderPassVulkan::RenderPassVulkan(VkDevice logicDevice, VkFormat imageFormat)
	:	mRenderPass(VK_NULL_HANDLE)
	{
		init(logicDevice, imageFormat);
	}

	void RenderPassVulkan::init(VkDevice logicDevice, VkFormat imageFormat) {
		VkAttachmentDescription colourAttachment = {};
		colourAttachment.format = imageFormat;
		colourAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colourAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colourAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colourAttachmentRef = {};
		colourAttachmentRef.attachment = 0;
		colourAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colourAttachmentRef;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPass = {};
		renderPass.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPass.attachmentCount = 1;
		renderPass.pAttachments = &colourAttachment;
		renderPass.subpassCount = 1;
		renderPass.pSubpasses = &subpass;
		renderPass.dependencyCount = 1;
		renderPass.pDependencies = &dependency;

		TRY(
			vkCreateRenderPass(logicDevice, &renderPass, nullptr, &mRenderPass) != VK_SUCCESS,
			"Failed to create Render Pass."
		);
	}

	void RenderPassVulkan::begin(VkFramebuffer framebuffer, VkExtent2D extent, VkCommandBuffer cmdBuffer) {
		VkRenderPassBeginInfo renderPassBegin = {};
		renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBegin.renderPass = mRenderPass;
		renderPassBegin.framebuffer = framebuffer;
		renderPassBegin.renderArea.offset = {0, 0};
		renderPassBegin.renderArea.extent = extent;

		VkClearValue clearColour = {0.0f, 0.0f, 0.0f, 1.0f};
		renderPassBegin.clearValueCount = 1;
		renderPassBegin.pClearValues = &clearColour;

		vkCmdBeginRenderPass(cmdBuffer, &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);
	}

	void RenderPassVulkan::close(VkDevice logicDevice) {
		vkDestroyRenderPass(logicDevice, mRenderPass, nullptr);
	}
}
