#pragma once

#include <vulkan/vulkan_core.h>

#include "dough/Core.h"
#include "dough/rendering/IGPUResourceVulkan.h"

namespace DOH {

	enum class ERenderPassAttachmentType {
		NONE = 0, //Use to "nullify" an attachment

		COLOUR,
		DEPTH
	};

	struct RenderPassAttachmentVulkan {
	
	public:
		const ERenderPassAttachmentType AttachmentType;
		const VkFormat Format;
		const VkAttachmentLoadOp LoadOp; //For LoadOp in Colour attachments, for DepthStencilLoadOp in DepthStencil attachments
		const VkAttachmentStoreOp StoreOp; //For StoreOp in Colour attachments, for DepthStencilStoreOp in DepthStencil attachments
		const VkImageLayout InitLayout;
		const VkImageLayout FinalLayout;
		const std::optional<VkClearValue> ClearValue;
	
		RenderPassAttachmentVulkan(
			const ERenderPassAttachmentType attachmentType,
			const VkFormat format,
			const VkAttachmentLoadOp loadOp,
			const VkAttachmentStoreOp storeOp,
			const VkImageLayout initLayout,
			const VkImageLayout finalLayout,
			const std::optional<VkClearValue> clearValue = {}
		) : AttachmentType(attachmentType),
			Format(format),
			LoadOp(loadOp),
			StoreOp(storeOp),
			InitLayout(initLayout),
			FinalLayout(finalLayout),
			ClearValue(clearValue)
		{}
	};
	
	struct SubPassVulkan {
	public:
		const VkPipelineBindPoint BindPoint;
		std::vector<RenderPassAttachmentVulkan> ColourAttachments;
		std::optional<RenderPassAttachmentVulkan> DepthStencilAttachment;
		//TODO::Whether cmds are inline or using secondary cmd buffers?

		SubPassVulkan(
			const VkPipelineBindPoint bindPoint,
			const std::initializer_list<RenderPassAttachmentVulkan> colourAttachments,
			const std::optional<RenderPassAttachmentVulkan> depthStencilAttachment = {}
		) : BindPoint(bindPoint),
			ColourAttachments(colourAttachments),
			DepthStencilAttachment(depthStencilAttachment)
		{}
		SubPassVulkan(const SubPassVulkan& copy) = delete;
		SubPassVulkan operator=(const SubPassVulkan& assignment) = delete;
	};

	class RenderPassVulkan : public IGPUResourceVulkan {

	private:
		
		//TODO:: Multiple subpass support
		//std::vector<SubPassVulkan> mSubPasses;

		VkRenderPass mRenderPass;
		std::vector<VkClearValue> mClearValues;

	public:
		RenderPassVulkan(
			VkDevice logicDevice,
			const SubPassVulkan& subPass
		);
		//RenderPassVulkan(
		//	VkDevice logicDevice,
		//	const std::vector<SubPassVulkan>& subPasses,
		//	const uint32_t colourAttachmentCount,
		//	const uint32_t depthAttachmentCount
		//);
		RenderPassVulkan(const RenderPassVulkan& copy) = delete;
		RenderPassVulkan operator=(const RenderPassVulkan& assignment) = delete;

		virtual void close(VkDevice logicDevice) override;

		void begin(
			VkFramebuffer frameBuffer,
			VkExtent2D extent,
			VkCommandBuffer cmd,
			bool inlineCommands = true
		);

		inline VkRenderPass get() const { return mRenderPass; }

		static void endRenderPass(VkCommandBuffer cmd) {
			vkCmdEndRenderPass(cmd);
		}
	};
}
