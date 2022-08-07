#include "dough/rendering/pipeline/RenderPassVulkan.h"

#include "dough/Utils.h"

#include <vector>

namespace DOH {

	RenderPassVulkan::RenderPassVulkan(
		VkDevice logicDevice,
		VkFormat imageFormat,
		VkImageLayout initialLayout,
		VkImageLayout finalLayout,
		VkAttachmentLoadOp loadOp,
		bool enableClearColour,
		VkClearValue clearColour,
		VkFormat depthFormat
	) : mRenderPass(VK_NULL_HANDLE),
		mClearColour(clearColour),
		mUsingDepthBuffer(false)
	{
		uint32_t attachmentIndex = 0;

		std::vector<VkAttachmentDescription> attachmentDescs = {};
		VkAttachmentDescription colourAttachment = {};
		colourAttachment.format = imageFormat;
		colourAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colourAttachment.loadOp = loadOp;
		colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colourAttachment.initialLayout = initialLayout;
		colourAttachment.finalLayout = finalLayout;

		VkAttachmentReference colourAttachmentRef = {};
		colourAttachmentRef.attachment = attachmentIndex;
		colourAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		attachmentDescs.push_back(colourAttachment);
		attachmentIndex++;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colourAttachmentRef;

		if (depthFormat != VK_FORMAT_UNDEFINED){
			VkAttachmentDescription depthAttachment = {};
			depthAttachment.format = depthFormat;
			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkAttachmentReference depthAttachRef = {};
			depthAttachRef.attachment = attachmentIndex;
			depthAttachRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			
			attachmentDescs.emplace_back(depthAttachment);
			attachmentIndex++;

			subpass.pDepthStencilAttachment = &depthAttachRef;

			mUsingDepthBuffer = true;
		}

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

		VkRenderPassCreateInfo renderPass = {};
		renderPass.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPass.attachmentCount = static_cast<uint32_t>(attachmentDescs.size());
		renderPass.pAttachments = attachmentDescs.data();
		renderPass.subpassCount = 1;
		renderPass.pSubpasses = &subpass;
		renderPass.dependencyCount = 1;
		renderPass.pDependencies = &dependency;

		VK_TRY(
			vkCreateRenderPass(logicDevice, &renderPass, nullptr, &mRenderPass),
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

		std::vector<VkClearValue> clearValues = { mClearColour };
		if (mUsingDepthBuffer) {
			VkClearValue depthClearValue = {};
			depthClearValue.depthStencil = { 1.0f, 0 };
			clearValues.emplace_back(depthClearValue);
		}

		renderPassBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassBegin.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(cmdBuffer, &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);
	}

	void RenderPassVulkan::close(VkDevice logicDevice) {
		vkDestroyRenderPass(logicDevice, mRenderPass, nullptr);
	}
}
