#include "dough/rendering/buffer/VertexBufferVulkan.h"

namespace DOH {

	VertexBufferVulkan::VertexBufferVulkan(
		const std::initializer_list<BufferElement>& elements,
		VkDevice logicDevice,
		VkPhysicalDevice physicalDevice,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags props
	) :	BufferVulkan(logicDevice, physicalDevice, size, usage, props),
		mBufferLayout(elements)
	{}

	//Staged
	VertexBufferVulkan::VertexBufferVulkan(
		const std::initializer_list<BufferElement>& elements,
		VkDevice logicDevice,
		VkPhysicalDevice physicalDevice,
		VkCommandPool cmdPool,
		VkQueue graphicsQueue,
		const void* data,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags props
	) : BufferVulkan(logicDevice, physicalDevice, cmdPool, graphicsQueue, data, size, usage, props),
		mBufferLayout(elements)
	{
		initStaged(logicDevice, physicalDevice, cmdPool, graphicsQueue, data, size, usage, props);
	}
}
