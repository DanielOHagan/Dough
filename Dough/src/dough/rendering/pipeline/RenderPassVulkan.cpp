#include "dough/rendering/pipeline/RenderPassVulkan.h"

#include "dough/Utils.h"

namespace DOH {

	RenderPassVulkan::RenderPassVulkan(VkDevice logicDevice, const SubPassVulkan& subPass) {
		uint32_t subPassAttachIndex = 0;
		const size_t colourAttachCount = subPass.ColourAttachments.size();

		std::vector<VkAttachmentDescription> attachDescs = {};
		std::vector<VkAttachmentReference> colourAttachRefs = {};

		if (colourAttachCount > 0) {
			attachDescs.reserve(colourAttachCount);
			colourAttachRefs.reserve(colourAttachCount);
		}

		for (const RenderPassAttachmentVulkan& colourAttach : subPass.ColourAttachments) {
			VkAttachmentDescription colourAttachDesc = {};
			colourAttachDesc.format = colourAttach.Format;
			colourAttachDesc.samples = VK_SAMPLE_COUNT_1_BIT;
			colourAttachDesc.loadOp = colourAttach.LoadOp;
			colourAttachDesc.storeOp = colourAttach.StoreOp;
			colourAttachDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colourAttachDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colourAttachDesc.initialLayout = colourAttach.InitLayout;
			colourAttachDesc.finalLayout = colourAttach.FinalLayout;
		
			VkAttachmentReference colourAttachRef = {};
			colourAttachRef.attachment = subPassAttachIndex;
			colourAttachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			attachDescs.emplace_back(colourAttachDesc);
			colourAttachRefs.emplace_back(colourAttachRef);
			subPassAttachIndex++;

			if (colourAttach.ClearValue.has_value()) {
				mClearValues.emplace_back(colourAttach.ClearValue.value());
			}
		}

		VkSubpassDescription subPassDesc = {};
		subPassDesc.pipelineBindPoint = subPass.BindPoint;
		subPassDesc.colorAttachmentCount = subPassAttachIndex;
		subPassDesc.pColorAttachments = colourAttachRefs.data();

		VkSubpassDependency subPassDependancy = {};
		subPassDependancy.srcSubpass = VK_SUBPASS_EXTERNAL;
		subPassDependancy.dstSubpass = 0;
		subPassDependancy.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subPassDependancy.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subPassDependancy.srcAccessMask = 0;
		subPassDependancy.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		subPassDependancy.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		if (subPass.DepthStencilAttachment.has_value()) {
			const RenderPassAttachmentVulkan& depth = subPass.DepthStencilAttachment.value();
			VkAttachmentDescription depthAttach = {};
			depthAttach.format = depth.Format;
			depthAttach.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttach.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttach.stencilLoadOp = depth.LoadOp;
			depthAttach.stencilStoreOp = depth.StoreOp;
			depthAttach.initialLayout = depth.InitLayout;
			depthAttach.finalLayout = depth.FinalLayout;

			VkAttachmentReference depthAttachRef = {};
			depthAttachRef.attachment = subPassAttachIndex;
			depthAttachRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; //  ??  depth.FinalLayout  ??  
		
			attachDescs.emplace_back(depthAttach);
			subPassAttachIndex++;

			subPassDesc.pDepthStencilAttachment = &depthAttachRef;

			if (depth.ClearValue.has_value()) {
				mClearValues.emplace_back(depth.ClearValue.value());
			}
		}

		VkRenderPassCreateInfo renderPass = {};
		renderPass.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPass.attachmentCount = subPassAttachIndex;
		renderPass.pAttachments = attachDescs.data();
		renderPass.subpassCount = 1;
		renderPass.pSubpasses = &subPassDesc;
		renderPass.dependencyCount = 1;
		renderPass.pDependencies = &subPassDependancy;

		VK_TRY(
			vkCreateRenderPass(logicDevice, &renderPass, nullptr, &mRenderPass),
			"Failed to create Render Pass."
		);
	}

	//TODO:: Multiple subpass support
	//RenderPassVulkan2::RenderPassVulkan2(
	//	VkDevice logicDevice,
	//	const std::vector<SubPassVulkan>& subPasses,
	//	const uint32_t colourAttachmentCount,
	//	const uint32_t depthAttachmentCount
	//) {
	//	//TODO:: How best to handle subPass dependencies?
	//
	//
	//	std::vector<VkSubpassDescription> subPassDescs = {};
	//	std::vector<VkSubpassDependency> subPassDependancies = {};
	//	std::vector<VkAttachmentDescription> attachDescs = {};
	//	std::vector<VkAttachmentReference> colourAttachRefs = {};
	//	std::vector<VkAttachmentReference> depthAttachRefs = {};
	//
	//	const size_t subPassCount = subPasses.size();
	//
	//	if (subPassCount == 0) {
	//		THROW("Render pass must contain at least one sub pass.");
	//	}
	//
	//	subPassDescs.reserve(subPassCount);
	//	attachDescs.reserve(colourAttachmentCount + depthAttachmentCount);
	//	colourAttachRefs.reserve(colourAttachmentCount);
	//	depthAttachRefs.reserve(depthAttachmentCount);
	//
	//	uint32_t subPassIndex = 0;
	//	uint32_t renderPassAttachIndex = 0;
	//	uint32_t depthAttachIndex = 0;
	//
	//	for (const SubPassVulkan& subPass : subPasses) {
	//		uint32_t subPassAttachIndex = 0;
	//
	//		for (const RenderPassAttachmentVulkan& colourAttach : subPass.ColourAttachments) {
	//			VkAttachmentDescription colourAttachDesc = {};
	//			colourAttachDesc.format = colourAttach.Format;
	//			colourAttachDesc.samples = VK_SAMPLE_COUNT_1_BIT;
	//			colourAttachDesc.loadOp = colourAttach.LoadOp;
	//			colourAttachDesc.storeOp = colourAttach.StoreOp;
	//			colourAttachDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	//			colourAttachDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	//			colourAttachDesc.initialLayout = colourAttach.InitLayout;
	//			colourAttachDesc.finalLayout = colourAttach.FinalLayout;
	//
	//			VkAttachmentReference colourAttachRef = {};
	//			colourAttachRef.attachment = renderPassAttachIndex + subPassAttachIndex;
	//			colourAttachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	//
	//			attachDescs.emplace_back(colourAttachDesc);
	//			colourAttachRefs.emplace_back(colourAttachRef);
	//			subPassAttachIndex++;
	//
	//			if (colourAttach.ClearValue.has_value()) {
	//				mClearValues.emplace_back(colourAttach.ClearValue.value());
	//			}
	//		}
	//
	//		VkSubpassDescription subPassDesc = {};
	//		subPassDesc.pipelineBindPoint = subPass.BindPoint;
	//		subPassDesc.colorAttachmentCount = subPassAttachIndex;
	//		subPassDesc.pColorAttachments = colourAttachRefs.data() + renderPassAttachIndex;
	//
	//		if (subPass.DepthStencilAttachment.has_value()) {
	//			const RenderPassAttachmentVulkan& depthAttach = subPass.DepthStencilAttachment.value();
	//			VkAttachmentDescription depthAttachDesc = {};
	//			depthAttachDesc.format = depthAttach.Format;
	//			depthAttachDesc.samples = VK_SAMPLE_COUNT_1_BIT;
	//			depthAttachDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	//			depthAttachDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	//			depthAttachDesc.stencilLoadOp = depthAttach.LoadOp;
	//			depthAttachDesc.stencilStoreOp = depthAttach.StoreOp;
	//			depthAttachDesc.initialLayout = depthAttach.InitLayout;
	//			depthAttachDesc.finalLayout = depthAttach.FinalLayout;
	//
	//			VkAttachmentReference depthAttachRef = {};
	//			depthAttachRef.attachment = renderPassAttachIndex + subPassAttachIndex;
	//			depthAttachRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; //  ??  depth.FinalLayout  ??  
	//
	//			attachDescs.emplace_back(depthAttachDesc);
	//			depthAttachRefs.emplace_back(depthAttachRef);
	//			subPassAttachIndex++;
	//
	//			subPassDesc.pDepthStencilAttachment = depthAttachRefs.data() + depthAttachIndex;
	//
	//			depthAttachIndex++;
	//
	//			if (depthAttach.ClearValue.has_value()) {
	//				mClearValues.emplace_back(depthAttach.ClearValue.value());
	//			}
	//		}
	//
	//
	//		//TODO::
	//		//if (subPass.hasDependancy()) {
	//		//	VkSubpassDependency dependancy = {};
	//		//
	//		//	subPassDependancies.emplace_back(dependancy);
	//		//}
	//
	//
	//		subPassDescs.emplace_back(subPassDesc);
	//
	//		renderPassAttachIndex += subPassAttachIndex;
	//		subPassIndex++;
	//	}
	//
	//	VkRenderPassCreateInfo renderPass = {};
	//	renderPass.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	//	renderPass.attachmentCount = static_cast<uint32_t>(attachDescs.size());
	//	renderPass.pAttachments = attachDescs.data();
	//	renderPass.subpassCount = static_cast<uint32_t>(subPassCount);
	//	renderPass.pSubpasses = subPassDescs.data();
	//	renderPass.dependencyCount = static_cast<uint32_t>(subPassDependancies.size());
	//	renderPass.pDependencies = subPassDependancies.data();
	//
	//	VK_TRY(
	//		vkCreateRenderPass(logicDevice, &renderPass, nullptr, &mRenderPass),
	//		"Failed to create Render Pass."
	//	);
	//}

	void RenderPassVulkan::begin(
		VkFramebuffer frameBuffer,
		VkExtent2D extent,
		VkCommandBuffer cmd,
		bool inlineCommands
	) {
		VkRenderPassBeginInfo renderPassBegin = {};
		renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBegin.renderPass = mRenderPass;
		renderPassBegin.framebuffer = frameBuffer;
		renderPassBegin.renderArea.offset = { 0, 0 };
		renderPassBegin.renderArea.extent = extent;

		renderPassBegin.clearValueCount = static_cast<uint32_t>(mClearValues.size());
		renderPassBegin.pClearValues = mClearValues.data();

		vkCmdBeginRenderPass(
			cmd,
			&renderPassBegin,
			inlineCommands ? VK_SUBPASS_CONTENTS_INLINE : VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS
		);
	}

	void RenderPassVulkan::close(VkDevice logicDevice) {
		vkDestroyRenderPass(logicDevice, mRenderPass, nullptr);
	}
}
