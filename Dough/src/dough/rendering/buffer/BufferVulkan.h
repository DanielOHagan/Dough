#pragma once

#include "dough/rendering/IGPUResourceVulkan.h"

namespace DOH {

	class BufferVulkan : public IGPUResourceVulkan {

	protected:
		VkBuffer mBuffer;
		VkDeviceMemory mBufferMemory;
		VkDeviceSize mSize;
		void* mData;

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

		virtual ~BufferVulkan() override;
		virtual void close(VkDevice logicDevice) override;

		//TODO::mem offset and flags
		void* map(VkDevice logicDevice, size_t size);
		void unmap(VkDevice logicDevice);
		void setDataUnmapped(VkDevice logicDevice, const void* data, size_t size);
		inline void setDataUnmapped(VkDevice logicDevice, void* data, size_t size) { setDataUnmapped(logicDevice, (const void*) data, size); }
		void setDataMapped(VkDevice logicDevice, const void* data, size_t size);
		inline void setDataMapped(VkDevice logicDevice, void* data, size_t size) { setDataMapped(logicDevice, (const void*) data, size); }
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
			resizeBufferStaged(
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

		inline VkBuffer getBuffer() const { return mBuffer; }
		inline VkDeviceMemory getDeviceMemory() const { return mBufferMemory; }
		inline size_t getSize() const { return mSize; }
		inline bool isMapped() const { return mData != nullptr; }

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
