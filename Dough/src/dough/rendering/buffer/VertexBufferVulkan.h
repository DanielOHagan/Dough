#pragma once

#include "dough/Utils.h"
#include "dough/rendering/Config.h"
#include "dough/rendering/buffer/BufferVulkan.h"
#include "dough/rendering/buffer/BufferLayout.h"


namespace DOH {

	class VertexBufferVulkan : public BufferVulkan {

	private:
		std::unique_ptr<BufferLayout> mBufferLayout;

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

		/** 
		* Close Vertex Buffer including Layout
		*/
		void close(VkDevice logicDevice) override;

		inline BufferLayout& getBufferLayout() const { return *mBufferLayout; }
	};
}
