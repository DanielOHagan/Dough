#include "dough/rendering/buffer/VertexBufferVulkan.h"

namespace DOH {

	//Non-Staged
	VertexBufferVulkan::VertexBufferVulkan(
		const AVertexInputLayout& vertexInputLayout,
		VkDevice logicDevice,
		VkPhysicalDevice physicalDevice,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags props
	) : BufferVulkan(logicDevice, physicalDevice, size, usage, props),
		mVertexInputLayout(vertexInputLayout)
	{}

	//Staged
	VertexBufferVulkan::VertexBufferVulkan(
		const AVertexInputLayout& vertexInputLayout,
		VkDevice logicDevice,
		VkPhysicalDevice physicalDevice,
		VkCommandPool cmdPool,
		VkQueue graphicsQueue,
		const void* data,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags props
	) : BufferVulkan(logicDevice, physicalDevice, cmdPool, graphicsQueue, data, size, usage, props),
		mVertexInputLayout(vertexInputLayout)
	{}

	VertexBufferVulkan::~VertexBufferVulkan() {
		if (isUsingGpuResource()) {
			LOG_ERR(
				"Vertex Array GPU resource NOT released before destructor was called." <<
				" Handle: " << mBuffer << " Device Memory: " << mBufferMemory << " Size: "<< mSize
			);
		}
	}

	void VertexBufferVulkan::close(VkDevice logicDevice) {
		BufferVulkan::close(logicDevice);
	}
}
