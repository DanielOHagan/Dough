#pragma once

#include <vulkan/vulkan_core.h>

namespace DOH {

	class BufferVulkan {

	protected:
		VkBuffer mBuffer;
		VkDeviceMemory mBufferMemory;
		VkDeviceSize mSize;

	public:
		BufferVulkan();

		void setData(VkDevice logicDevice, const void* data, size_t size);
		void setData(VkDevice logicDevice, void* data, size_t size) { setData(logicDevice, (const void*) data, size); }

		void copyToBuffer(BufferVulkan& destination);
		void copyFromBuffer(BufferVulkan& source);

		void close(VkDevice logicDevice);

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
		static BufferVulkan createBuffer(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		);

		/**
		* Create a buffer that uses a staging buffer to transfer
		*  data to GPU memory.
		* 
		* VK_BUFFER_USAGE_TRANSFER_DST_BIT automatically added 
		*  to usage param in method.
		* 
		*/
		static BufferVulkan createStagedBuffer(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkCommandPool cmdPool,
			VkQueue graphicsQueue,
			const void* data,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		);

		static BufferVulkan createStagedBuffer(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkCommandPool cmdPool,
			VkQueue graphicsQueue,
			void* data,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		) {
			return createStagedBuffer(
				logicDevice,
				physicalDevice,
				cmdPool,
				graphicsQueue,
				(const void*) data,
				size,
				usage,
				props
			);
		}

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
