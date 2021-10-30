#include "dough/rendering/buffer/VertexArrayVulkan.h"

namespace DOH {

	VertexArrayVulkan::VertexArrayVulkan() {}

	void VertexArrayVulkan::bind(VkCommandBuffer cmdBuffer) {
		std::vector<VkBuffer> vertexBuffers;
		std::vector<VkDeviceSize> offsets{0};
		for (std::shared_ptr<VertexBufferVulkan> vertexBuffer : mVertexBuffers) {
			vertexBuffers.push_back(vertexBuffer->getBuffer());
			offsets.push_back(static_cast<VkDeviceSize>(vertexBuffer->getBufferLayout().getStride()));
		}
		vkCmdBindVertexBuffers(cmdBuffer, 0, static_cast<uint32_t>(vertexBuffers.size()), vertexBuffers.data(), offsets.data());
		vkCmdBindIndexBuffer(cmdBuffer, mIndexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT16);
	}

	void VertexArrayVulkan::addVertexBuffer(std::shared_ptr<VertexBufferVulkan> vertexBuffer) {
		mVertexBuffers.push_back(vertexBuffer);
	}

	void VertexArrayVulkan::close(VkDevice logicDevice) {
		for (std::shared_ptr<VertexBufferVulkan> vertexBuffer : mVertexBuffers) {
			vertexBuffer->close(logicDevice);
		}

		mIndexBuffer->close(logicDevice);
	}
}
