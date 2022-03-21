#pragma once

#include <vulkan/vulkan_core.h>

namespace DOH {

	//TODO:: Maybe have an enum for renderpass position or pass in init & final layout, instead of using bools
	class RenderPassVulkan {

	private:
		VkRenderPass mRenderPass;
		VkClearValue mClearColour;
		uint32_t mClearCount;

	public:
		//TODO:: Make a better and less confusing way instead of using loads of bools
		RenderPassVulkan(
			VkDevice logicDevice,
			VkFormat imageFormat,
			bool hasPassBefore,
			bool hasPassAfter,
			bool enableClearColour,
			VkClearValue clearColour = { 0.264f, 0.328f, 0.484f, 1.0f }
		);
		RenderPassVulkan(const RenderPassVulkan& copy) = delete;
		RenderPassVulkan operator=(const RenderPassVulkan& assignment) = delete;

		void begin(VkFramebuffer framebuffer, VkExtent2D extent, VkCommandBuffer cmdBuffer);
		void close(VkDevice logicDevice);

		inline void setClearColour(float r, float g, float b, float a) { mClearColour = { r, g, b, a }; }
		inline VkRenderPass get() const { return mRenderPass; }
	};
}
