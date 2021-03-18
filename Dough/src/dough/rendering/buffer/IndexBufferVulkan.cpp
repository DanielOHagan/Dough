#include "dough/rendering/buffer/IndexBufferVulkan.h"

namespace DOH {

	IndexBufferVulkan::IndexBufferVulkan(uint32_t count)
	:	BufferVulkan(),
		mCount(count)
	{
	}

	IndexBufferVulkan IndexBufferVulkan::createIndexBuffer(
		VkDevice logicDevice,
		VkPhysicalDevice physicalDevice,
		VkDeviceSize size,
		uint32_t count
	) {
		IndexBufferVulkan indexBuffer = IndexBufferVulkan(count);
		indexBuffer.init(
			logicDevice,
			physicalDevice,
			size,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);
		return indexBuffer;
	}

	IndexBufferVulkan IndexBufferVulkan::createStagedIndexBuffer(
		VkDevice logicDevice,
		VkPhysicalDevice physicalDevice,
		VkCommandPool cmdPool,
		VkQueue graphicsQueue,
		const void* data,
		VkDeviceSize size,
		uint32_t count
	) {
		IndexBufferVulkan indexBuffer = IndexBufferVulkan(count);
		indexBuffer.initStaged(
			logicDevice,
			physicalDevice,
			cmdPool,
			graphicsQueue,
			data,
			size,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		return indexBuffer;
	}
}