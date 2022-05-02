#pragma once

#include "dough/rendering/buffer/BufferVulkan.h"

namespace DOH {

	class IndexBufferVulkan : public BufferVulkan {

	public:
		IndexBufferVulkan() = delete;
		IndexBufferVulkan(const IndexBufferVulkan& copy) = delete;
		IndexBufferVulkan operator=(const IndexBufferVulkan& assignment) = delete;

		//Non-Staged
		IndexBufferVulkan(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkDeviceSize size
		);
		//Staged
		IndexBufferVulkan(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkCommandPool cmdPool,
			VkQueue graphicsQueue,
			const void* data,
			VkDeviceSize size
		);

		void bind(VkCommandBuffer cmd);
	};
}
