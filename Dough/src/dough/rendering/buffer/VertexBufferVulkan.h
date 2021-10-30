#pragma once

#include "dough/Utils.h"
#include "dough/rendering/Config.h"
#include "dough/rendering/buffer/BufferVulkan.h"
#include "dough/rendering/buffer/BufferLayout.h"


namespace DOH {

	class VertexBufferVulkan : public BufferVulkan {

	private:
		BufferLayout mBufferLayout;

	public:
		VertexBufferVulkan(const std::initializer_list<BufferElement>& elements);

		inline BufferLayout getBufferLayout() const { return mBufferLayout; }
		inline void setBufferLayout(const BufferLayout& bufferLayout) { mBufferLayout = bufferLayout; }

		static VertexBufferVulkan createStagedVertexBuffer(
			const std::initializer_list<BufferElement>& elements,
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkCommandPool cmdPool,
			VkQueue graphicsQueue,
			const void* data,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		);

		static VertexBufferVulkan createStagedVertexBuffer(
			const std::initializer_list<BufferElement>& elements,
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkCommandPool cmdPool,
			VkQueue graphicsQueue,
			void* data,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		) {
			return createStagedVertexBuffer(
				elements,
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
	};
}
