#pragma once

#include <vulkan/vulkan_core.h>

namespace DOH {

	class BufferVulkan {

	private:
		VkBuffer mBuffer;
		VkDeviceMemory mBufferMemory;

	public:
		BufferVulkan();

		void init(
			VkDevice logicDevice,
			VkPhysicalDevice phyiscalDevice,
			size_t sizeOfDataBytes,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		);
		void close(VkDevice logicDevice);
		
		void setData(VkDevice logicDevice, const void* data, size_t size);
		void setData(VkDevice logicDevice, void* data, size_t size) { setData(logicDevice, (const void*) data, size); }

		VkBuffer getBuffer() const { return mBuffer; }
		VkDeviceMemory getDeviceMemory() const { return mBufferMemory; }


		static BufferVulkan createBuffer(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		);
	};
}