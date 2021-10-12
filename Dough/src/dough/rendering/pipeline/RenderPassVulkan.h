#pragma once

#include "dough/Utils.h"

#include <vulkan/vulkan_core.h>

namespace DOH {

	class RenderPassVulkan {

	private:
		VkRenderPass mRenderPass;

	private:
		RenderPassVulkan();

		void init(VkDevice logicDevice, VkFormat imageFormat);

	public:

		void begin(VkFramebuffer framebuffer, VkExtent2D extent, VkCommandBuffer cmdBuffer);
		void close(VkDevice logicDevice);

		inline VkRenderPass get() const { return mRenderPass; }

	public:
		static RenderPassVulkan create(VkDevice logicDevice, VkFormat imageFormat);

		static RenderPassVulkan createNonInit() { return RenderPassVulkan(); }
	};
}
