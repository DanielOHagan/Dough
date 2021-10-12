#pragma once

#include "dough/rendering/buffer/BufferVulkan.h"

namespace DOH {

	class IndexBufferVulkan : public BufferVulkan {

	private:
		uint32_t mCount;

	public:
		IndexBufferVulkan(uint32_t count = 0);

		inline uint32_t getCount() const { return mCount; }

	public:

		static IndexBufferVulkan createIndexBuffer(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkDeviceSize size,
			uint32_t count
		);

		static IndexBufferVulkan createStagedIndexBuffer(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkCommandPool cmdPool,
			VkQueue graphicsQueue,
			const void* data,
			VkDeviceSize size,
			uint32_t count
		);
	};
}
