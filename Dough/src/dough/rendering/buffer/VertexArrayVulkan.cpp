#include "dough/rendering/buffer/VertexArrayVulkan.h"

#include "dough/Logging.h"

namespace DOH {

	const uint32_t VertexArrayVulkan::MAX_VBO_COUNT = 1;

	void VertexArrayVulkan::bind(VkCommandBuffer cmdBuffer) {
		std::vector<VkBuffer> vertexBuffers;
		std::vector<VkDeviceSize> offsets{0};
		for (std::shared_ptr<VertexBufferVulkan> vertexBuffer : mVertexBuffers) {
			vertexBuffers.push_back(vertexBuffer->getBuffer());
			offsets.push_back(static_cast<VkDeviceSize>(vertexBuffer->getBufferLayout().getStride()));
		}
		vkCmdBindVertexBuffers(
			cmdBuffer,
			0,
			static_cast<uint32_t>(vertexBuffers.size()),
			vertexBuffers.data(),
			offsets.data()
		);
		vkCmdBindIndexBuffer(cmdBuffer, mIndexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT16);
	}

	void VertexArrayVulkan::addVertexBuffer(std::shared_ptr<VertexBufferVulkan> vertexBuffer) {
		const uint32_t vboCount = static_cast<uint32_t>(mVertexBuffers.size());
		if (vboCount + 1 <= VertexArrayVulkan::MAX_VBO_COUNT) {
			mVertexBuffers.push_back(vertexBuffer);
		} else {
			LOG_WARN(
				"Failed to add VBO to VAO. Adding another would exceed max of "
				<< VertexArrayVulkan::MAX_VBO_COUNT
			);
		}
	}

	void VertexArrayVulkan::close(VkDevice logicDevice) {
		for (std::shared_ptr<VertexBufferVulkan> vertexBuffer : mVertexBuffers) {
			vertexBuffer->close(logicDevice);
		}

		mIndexBuffer->close(logicDevice);

		mUsingGpuResource = false;
	}

	bool VertexArrayVulkan::isUsingGpuResource() const {
		for (std::shared_ptr<VertexBufferVulkan> vertexBuffer : mVertexBuffers) {
			if (vertexBuffer->isUsingGpuResource()) {
				return true;
			}
		}

		return mIndexBuffer->isUsingGpuResource();
	}
}
