#pragma once

#include "dough/Utils.h"
#include "dough/rendering/Config.h"
#include "dough/rendering/buffer/BufferVulkan.h"
#include "dough/rendering/VertexInputLayout.h"


namespace DOH {

	class VertexBufferVulkan : public BufferVulkan {

	private:
		const AVertexInputLayout& mVertexInputLayout;

	public:
		VertexBufferVulkan() = delete;
		VertexBufferVulkan(const VertexBufferVulkan& copy) = delete;
		VertexBufferVulkan operator=(const VertexBufferVulkan& assignment) = delete;

		//Non-Staged
		VertexBufferVulkan(
			const AVertexInputLayout& vertexInputLayout,
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		);
		//Staged
		VertexBufferVulkan(
			const AVertexInputLayout& vertexInputLayout,
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkCommandPool cmdPool,
			VkQueue graphicsQueue,
			const void* data,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		);
		
		virtual ~VertexBufferVulkan() override;

		virtual void close(VkDevice logicDevice) override;

		inline const AVertexInputLayout& getVertexInputLayout() const { return mVertexInputLayout; }
	};
}
