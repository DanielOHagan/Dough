#include "dough/rendering/buffer/BufferVulkan.h"

#include "dough/rendering/RendererVulkan.h"
#include "dough/application/Application.h"

namespace DOH {

	const BufferVulkan* BufferVulkan::EMPTY_BUFFER = nullptr;

	//Non-staged
	BufferVulkan::BufferVulkan(
		VkDevice logicDevice,
		VkPhysicalDevice physicalDevice,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags props
	) : mBuffer(VK_NULL_HANDLE),
		mBufferMemory(VK_NULL_HANDLE),
		mSize(size)
	{
		if (size > 0) {
			init(logicDevice, physicalDevice, size, usage, props);
		}
	}

	//Staged
	BufferVulkan::BufferVulkan(
		VkDevice logicDevice,
		VkPhysicalDevice physicalDevice,
		VkCommandPool cmdPool,
		VkQueue graphicsQueue,
		const void* data,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags props
	) : mBuffer(VK_NULL_HANDLE),
		mBufferMemory(VK_NULL_HANDLE),
		mSize(size)
	{
		if (size > 0) {
			initStaged(logicDevice, physicalDevice, cmdPool, graphicsQueue, data, size, usage, props);
		}
	}

	void BufferVulkan::init(
		VkDevice logicDevice,
		VkPhysicalDevice physicalDevice,
		size_t size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags props
	) {
		mSize = size;

		VkBufferCreateInfo bufferCreateInfo = {};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = size;
		bufferCreateInfo.usage = usage;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VK_TRY(
			vkCreateBuffer(logicDevice, &bufferCreateInfo, nullptr, &mBuffer),
			"Failed to create Vertex Buffer."
		);

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(logicDevice, mBuffer, &memRequirements);

		VkMemoryAllocateInfo allocation = {};
		allocation.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocation.allocationSize = memRequirements.size;
		allocation.memoryTypeIndex = RendererVulkan::findPhysicalDeviceMemoryType(
			physicalDevice,
			memRequirements.memoryTypeBits,
			props
		);

		VK_TRY(
			vkAllocateMemory(logicDevice, &allocation, nullptr, &mBufferMemory),
			"Failed to allocate buffer memory."
		);

		vkBindBufferMemory(logicDevice, mBuffer, mBufferMemory, 0);
	}

	void BufferVulkan::initStaged(
		VkDevice logicDevice,
		VkPhysicalDevice physicalDevice,
		VkCommandPool cmdPool,
		VkQueue graphicsQueue,
		const void* data,
		size_t size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags props
	) {
		mSize = size;

		//Add transfer destination bit to usage if not included already
		usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		BufferVulkan stagingBuffer = BufferVulkan(
			logicDevice,
			physicalDevice,
			size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);
		stagingBuffer.setData(logicDevice, data, size);

		init(logicDevice, physicalDevice, size, usage, props);
		BufferVulkan::copyBuffer(logicDevice, cmdPool, graphicsQueue, stagingBuffer.getBuffer(), mBuffer, size);

		stagingBuffer.close(logicDevice);
	}

	void BufferVulkan::resizeBuffer(
		VkDevice logicDevice,
		VkPhysicalDevice physicalDevice,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags props
	) {
		clearBuffer(logicDevice);
		init(logicDevice, physicalDevice, size, usage, props);
	}

	void BufferVulkan::resizeBufferStaged(
		VkDevice logicDevice,
		VkPhysicalDevice physicalDevice,
		VkCommandPool cmdPool,
		VkQueue graphicsQueue,
		const void* data,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags props
	) {
		clearBuffer(logicDevice);
		initStaged(logicDevice, physicalDevice, cmdPool, graphicsQueue, data, size, usage, props);
	}

	void BufferVulkan::close(VkDevice logicDevice) {
		clearBuffer(logicDevice);
	}

	void BufferVulkan::setData(VkDevice logicDevice, const void* data, size_t size) {
		mSize = size;
		void* mappedData;
		vkMapMemory(logicDevice, mBufferMemory, 0, size, 0, &mappedData);
		memcpy(mappedData, data, size);
		vkUnmapMemory(logicDevice, mBufferMemory);
	}

	void BufferVulkan::clearBuffer(VkDevice logicDevice) {
		vkDestroyBuffer(logicDevice, mBuffer, nullptr);
		vkFreeMemory(logicDevice, mBufferMemory, nullptr);
	}

	void BufferVulkan::copyToBuffer(BufferVulkan& destination, VkCommandBuffer cmd) {
		BufferVulkan::copyBuffer(*this, destination, cmd);
	}

	void BufferVulkan::copyFromBuffer(BufferVulkan& source, VkCommandBuffer cmd) {
		BufferVulkan::copyBuffer(source, *this, cmd);
	}

	void BufferVulkan::copyBuffer(
		VkDevice logicDevice,
		VkCommandPool cmdPool,
		VkQueue graphicsQueue,
		VkBuffer srcBuffer,
		VkBuffer dstBuffer,
		VkDeviceSize size
	) {
		VkCommandBufferAllocateInfo allocation = {};
		allocation.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocation.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocation.commandPool = cmdPool;
		allocation.commandBufferCount = 1;

		VkCommandBuffer cmdBuffer;
		vkAllocateCommandBuffers(logicDevice, &allocation, &cmdBuffer);

		VkCommandBufferBeginInfo cmdBegin = {};
		cmdBegin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBegin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(cmdBuffer, &cmdBegin);
		
		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = size;
		vkCmdCopyBuffer(cmdBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		vkEndCommandBuffer(cmdBuffer);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;

		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue);

		vkFreeCommandBuffers(logicDevice, cmdPool, 1, &cmdBuffer);
	}

	void BufferVulkan::copyBuffer(BufferVulkan& source, BufferVulkan& destination, VkCommandBuffer cmd) {
		VkBufferCopy copy{};
		copy.size = source.getSize();
		vkCmdCopyBuffer(cmd, source.getBuffer(), destination.getBuffer(), 1, &copy);
	}

	void BufferVulkan::copyToImage(VkCommandBuffer cmd, VkImage dstImage, VkImageLayout dstImageLayout, VkBufferImageCopy& region) {
		vkCmdCopyBufferToImage(cmd, mBuffer, dstImage, dstImageLayout, 1, &region);
	}
}
