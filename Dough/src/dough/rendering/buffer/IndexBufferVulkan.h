#pragma once

#include "dough/rendering/buffer/BufferVulkan.h"

namespace DOH {

	class IndexBufferVulkan : public BufferVulkan {

	private:
		uint32_t mCount;

	public:
		IndexBufferVulkan() = delete;
		IndexBufferVulkan(const IndexBufferVulkan& copy) = delete;
		IndexBufferVulkan operator=(const IndexBufferVulkan& assignment) = delete;

		//Non-Staged
		IndexBufferVulkan(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkDeviceSize size,
			uint32_t count
		);
		//Staged
		IndexBufferVulkan(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkCommandPool cmdPool,
			VkQueue graphicsQueue,
			const void* data,
			VkDeviceSize size,
			uint32_t count
		);

		inline uint32_t getCount() const { return mCount; }
		inline void setCount(uint32_t count) { mCount = count; }
	};
}
