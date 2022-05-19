#include "dough/rendering/buffer/VertexBufferVulkan.h"

namespace DOH {

	VertexBufferVulkan::VertexBufferVulkan(
		const std::initializer_list<BufferElement>& elements,
		VkDevice logicDevice,
		VkPhysicalDevice physicalDevice,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags props
	) :	BufferVulkan(
			logicDevice,
			physicalDevice,
			size,
			usage,
			props
		),
		mBufferLayout(std::make_unique<BufferLayout>(elements))
	{}

	VertexBufferVulkan::VertexBufferVulkan(
		const std::vector<BufferElement>& elements,
		VkDevice logicDevice,
		VkPhysicalDevice physicalDevice,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags props
	) : BufferVulkan(
			logicDevice,
			physicalDevice,
			size,
			usage,
			props
		),
		mBufferLayout(std::make_unique<BufferLayout>(elements))
	{}

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
	) : BufferVulkan(
			logicDevice,
			physicalDevice,
			cmdPool,
			graphicsQueue,
			data,
			size,
			usage,
			props
		),
		mBufferLayout(std::make_unique<BufferLayout>(elements))
	{}
	
	VertexBufferVulkan::VertexBufferVulkan(
		const std::vector<BufferElement>& elements,
		VkDevice logicDevice,
		VkPhysicalDevice physicalDevice,
		VkCommandPool cmdPool,
		VkQueue graphicsQueue,
		const void* data,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags props
	) : BufferVulkan(
			logicDevice,
			physicalDevice,
			cmdPool,
			graphicsQueue,
			data,
			size,
			usage,
			props
		),
		mBufferLayout(std::make_unique<BufferLayout>(elements))
	{}

	VertexBufferVulkan::VertexBufferVulkan(
		const EVertexType vertexType,
		VkDevice logicDevice,
		VkPhysicalDevice physicalDevice,
		VkCommandPool cmdPool,
		VkQueue graphicsQueue,
		const void* data,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags props
	) : BufferVulkan(
			logicDevice,
			physicalDevice,
			cmdPool,
			graphicsQueue,
			data,
			size,
			usage,
			props
		),
		mBufferLayout(std::make_unique<BufferLayout>(vertexType))
	{}

	void VertexBufferVulkan::close(VkDevice logicDevice) {
		BufferVulkan::close(logicDevice);
		mBufferLayout.reset();
	}
}
