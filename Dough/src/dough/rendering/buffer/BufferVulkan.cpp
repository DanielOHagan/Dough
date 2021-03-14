#include "dough/rendering/buffer/BufferVulkan.h"

#include "dough/rendering/RenderingContextVulkan.h"

namespace DOH {

	BufferVulkan::BufferVulkan()
	:	mBuffer(VK_NULL_HANDLE),
		mBufferMemory(VK_NULL_HANDLE)
	{
	}

	void BufferVulkan::init(
		VkDevice logicDevice,
		VkPhysicalDevice physicalDevice,
		size_t sizeOfDataBytes,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags props
	) {
		VkBufferCreateInfo bufferCreateInfo = {};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = sizeOfDataBytes;
		bufferCreateInfo.usage = usage;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		TRY(
			vkCreateBuffer(logicDevice, &bufferCreateInfo, nullptr, &mBuffer) != VK_SUCCESS,
			"Failed to create Vertex Buffer."
		);

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(logicDevice, mBuffer, &memRequirements);

		VkMemoryAllocateInfo allocation = {};
		allocation.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocation.allocationSize = memRequirements.size;
		allocation.memoryTypeIndex = RenderingContextVulkan::findPhysicalDeviceMemoryType(
			physicalDevice,
			memRequirements.memoryTypeBits,
			props
		);

		TRY(
			vkAllocateMemory(logicDevice, &allocation, nullptr, &mBufferMemory) != VK_SUCCESS,
			"Failed to allocate buffer memory."
		);

		vkBindBufferMemory(logicDevice, mBuffer, mBufferMemory, 0);
	}

	void BufferVulkan::close(VkDevice logicDevice) {
		vkDestroyBuffer(logicDevice, mBuffer, nullptr);
		vkFreeMemory(logicDevice, mBufferMemory, nullptr);
	}

	void BufferVulkan::setData(VkDevice logicDevice, const void* data, size_t size) {
		void* mappedData;
		vkMapMemory(logicDevice, mBufferMemory, 0, size, 0, &mappedData);
		memcpy(mappedData, data, size);
		vkUnmapMemory(logicDevice, mBufferMemory);
	}

	BufferVulkan BufferVulkan::createBuffer(
		VkDevice logicDevice,
		VkPhysicalDevice physicalDevice,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags props
	) {
		BufferVulkan buffer = BufferVulkan();
		buffer.init(logicDevice, physicalDevice, (uint32_t) size, usage, props);
		return buffer;
	}
}