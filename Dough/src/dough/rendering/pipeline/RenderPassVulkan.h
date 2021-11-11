#pragma once

#include <vulkan/vulkan_core.h>

namespace DOH {

	class RenderPassVulkan {

	private:
		VkRenderPass mRenderPass;
		VkClearValue mClearColour;

	private:
		void init(VkDevice logicDevice, VkFormat imageFormat);

	public:
		RenderPassVulkan(VkDevice logicDevice, VkFormat imageFormat);
		RenderPassVulkan(const RenderPassVulkan& copy) = delete;
		RenderPassVulkan operator=(const RenderPassVulkan& assignment) = delete;

		void begin(VkFramebuffer framebuffer, VkExtent2D extent, VkCommandBuffer cmdBuffer);
		void close(VkDevice logicDevice);

		inline void setClearColour(float r, float g, float b, float a) { mClearColour = { r, g, b, a }; }
		inline VkRenderPass get() const { return mRenderPass; }
	};
}
