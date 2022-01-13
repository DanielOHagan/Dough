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

		void copyToBuffer(BufferVulkan& destination);
		void copyFromBuffer(BufferVulkan& source);

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

		static void copyBuffer(BufferVulkan& source, BufferVulkan& destination);
	};
}
