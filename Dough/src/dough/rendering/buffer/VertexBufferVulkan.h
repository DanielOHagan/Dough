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
		VertexBufferVulkan() = delete;
		VertexBufferVulkan(const VertexBufferVulkan& copy) = delete;
		VertexBufferVulkan operator=(const VertexBufferVulkan& assignment) = delete;

		//Non-Staged
		VertexBufferVulkan(
			const std::initializer_list<BufferElement>& elements,
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		);

		//Staged
		VertexBufferVulkan(
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

		inline BufferLayout getBufferLayout() const { return mBufferLayout; }
		inline void setBufferLayout(const BufferLayout& bufferLayout) { mBufferLayout = bufferLayout; }
	};
}
