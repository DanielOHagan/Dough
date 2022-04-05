#pragma once

#include "dough/rendering/IGPUResourceVulkan.h"

namespace DOH {

	class BufferVulkan : public IGPUResourceVulkan {

	protected:
		VkBuffer mBuffer;
		VkDeviceMemory mBufferMemory;
		VkDeviceSize mSize;

	public:

		BufferVulkan() = delete;
		BufferVulkan(const BufferVulkan& copy) = delete;
		BufferVulkan operator=(const BufferVulkan& assignment) = delete;

		~BufferVulkan();

		//Non-Staged
		BufferVulkan(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		);
		//Staged
		BufferVulkan(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkCommandPool cmdPool,
			VkQueue graphicsQueue,
			const void* data,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		);

		void setData(VkDevice logicDevice, const void* data, size_t size);
		void setData(VkDevice logicDevice, void* data, size_t size) { setData(logicDevice, (const void*) data, size); }
		void clearBuffer(VkDevice logicDevice);

		void copyToBuffer(BufferVulkan& destination, VkCommandBuffer cmd);
		void copyFromBuffer(BufferVulkan& source, VkCommandBuffer cmd);
		void copyToImage(VkCommandBuffer cmd, VkImage dstImage, VkImageLayout dstImageLayout, VkBufferImageCopy& region);

		void resizeBuffer(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		);
		void resizeBufferStaged(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkCommandPool cmdPool,
			VkQueue graphicsQueue,
			const void* data,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		);
		inline void resizeBufferStaged(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkCommandPool cmdPool,
			VkQueue graphicsQueue,
			void* data,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		) {
			resizeBufferStaged(logicDevice, physicalDevice, cmdPool, graphicsQueue, (const void*)data, size, usage, props);
		}
		virtual void close(VkDevice logicDevice) override;

		inline VkBuffer getBuffer() const { return mBuffer; }
		inline VkDeviceMemory getDeviceMemory() const { return mBufferMemory; }
		inline size_t getSize() const { return mSize; }

	protected:
		void init(
			VkDevice logicDevice,
			VkPhysicalDevice phyiscalDevice,
			size_t size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		);
		void initStaged(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkCommandPool cmdPool,
			VkQueue graphicsQueue,
			const void* data,
			size_t size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		);

	public:
		static void copyBuffer(
			VkDevice logicDevice,
			VkCommandPool cmdPool,
			VkQueue graphicsQueue,
			VkBuffer srcBuffer,
			VkBuffer dstBuffer,
			VkDeviceSize size
		);

		static void copyBuffer(BufferVulkan& source, BufferVulkan& destination, VkCommandBuffer cmd);
	};
}
