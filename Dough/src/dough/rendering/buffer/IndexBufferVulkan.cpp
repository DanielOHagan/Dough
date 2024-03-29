#include "dough/rendering/buffer/IndexBufferVulkan.h"

namespace DOH {

	//Non-Staged
	IndexBufferVulkan::IndexBufferVulkan(
		VkDevice logicDevice,
		VkPhysicalDevice physicalDevice,
		VkDeviceSize size
	) : BufferVulkan(
		logicDevice,
		physicalDevice,
		size,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	) {}

	//Staged
	IndexBufferVulkan::IndexBufferVulkan(
		VkDevice logicDevice,
		VkPhysicalDevice physicalDevice,
		VkCommandPool cmdPool,
		VkQueue graphicsQueue,
		const void* data,
		VkDeviceSize size
	) : BufferVulkan(
		logicDevice,
		physicalDevice,
		cmdPool,
		graphicsQueue,
		data,
		size,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	) {}

	IndexBufferVulkan::~IndexBufferVulkan() {
		//GPU resources MUST already be released separately from destructor
		if (isUsingGpuResource()) {
			LOG_ERR(
				"Index Buffer GPU resource NOT released before destructor was called." << 
				" Handle: " << mBuffer << " Memory: " << mBufferMemory << " Size: " << mSize
			);

			//TODO:: some kind of "lost GPU resources" list to manage?
		}
	}

	void IndexBufferVulkan::bind(VkCommandBuffer cmd) {
		vkCmdBindIndexBuffer(cmd, mBuffer, 0, VK_INDEX_TYPE_UINT32);
	}
}
