#include "dough/rendering/buffer/VertexBufferVulkan.h"

namespace DOH {

	VertexBufferVulkan::VertexBufferVulkan(const std::initializer_list<BufferElement>& elements) 
	:	BufferVulkan(),
		mBufferLayout(elements)
	{}

	VertexBufferVulkan VertexBufferVulkan::createStagedVertexBuffer(
		const std::initializer_list<BufferElement>& elements,
		VkDevice logicDevice,
		VkPhysicalDevice physicalDevice,
		VkCommandPool cmdPool,
		VkQueue graphicsQueue,
		const void* data,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags props
	) {
		VertexBufferVulkan vBuffer = VertexBufferVulkan(elements);
		vBuffer.initStaged(logicDevice, physicalDevice, cmdPool, graphicsQueue, data, size, usage, props);
		return vBuffer;
	}
}
