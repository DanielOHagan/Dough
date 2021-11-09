#include "dough/rendering/buffer/IndexBufferVulkan.h"

namespace DOH {

	//Non-Staged
	IndexBufferVulkan::IndexBufferVulkan(
		VkDevice logicDevice,
		VkPhysicalDevice physicalDevice,
		VkDeviceSize size,
		uint32_t count
	) : BufferVulkan(
			logicDevice,
			physicalDevice,
			size,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		),
		mCount(count)
	{}

	//Staged
	IndexBufferVulkan::IndexBufferVulkan(
		VkDevice logicDevice,
		VkPhysicalDevice physicalDevice,
		VkCommandPool cmdPool,
		VkQueue graphicsQueue,
		const void* data,
		VkDeviceSize size,
		uint32_t count
	) : BufferVulkan(
			logicDevice,
			physicalDevice,
			cmdPool,
			graphicsQueue,
			data,
			size,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		),
		mCount(count)
	{}
}
