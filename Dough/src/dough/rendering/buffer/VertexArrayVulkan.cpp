#include "dough/rendering/buffer/VertexArrayVulkan.h"

#include "dough/Logging.h"

namespace DOH {

	const uint32_t VertexArrayVulkan::MAX_VBO_COUNT = 1;

	VertexArrayVulkan::VertexArrayVulkan()
	:	mIndexBuffer(nullptr),
		mDrawCount(0),
		mSharingIndexBuffer(false),
		mPushConstantData(nullptr)
	{}

	void VertexArrayVulkan::bind(VkCommandBuffer cmd) {
		std::vector<VkBuffer> vertexBuffers;
		std::vector<VkDeviceSize> offsets;

		const size_t vbCount = mVertexBuffers.size();
		vertexBuffers.resize(vbCount);
		offsets.resize(vbCount + 1);
		offsets[0] = 0;

		for (size_t i = 0; i < vbCount; i++) {
			vertexBuffers[i] = mVertexBuffers[i]->getBuffer();
			offsets[i + 1] = static_cast<VkDeviceSize>(mVertexBuffers[i]->getBufferLayout().getStride());
		}

		vkCmdBindVertexBuffers(
			cmd,
			0,
			static_cast<uint32_t>(vbCount),
			vertexBuffers.data(),
			offsets.data()
		);

		if (!mSharingIndexBuffer) {
			mIndexBuffer->bind(cmd);
		}
	}

	void VertexArrayVulkan::addVertexBuffer(std::shared_ptr<VertexBufferVulkan> vertexBuffer) {
		const uint32_t vboCount = static_cast<uint32_t>(mVertexBuffers.size());
		if (vboCount + 1 > VertexArrayVulkan::MAX_VBO_COUNT) {
			LOG_WARN(
				"Failed to add VBO to VAO. Adding another would exceed max of "
				<< VertexArrayVulkan::MAX_VBO_COUNT
			);
		} else {
			mVertexBuffers.push_back(vertexBuffer);
		}
	}

	void VertexArrayVulkan::closeVertexBuffers(VkDevice logicDevice) {
		for (std::shared_ptr<VertexBufferVulkan> vbo : mVertexBuffers) {
			vbo->close(logicDevice);
		}

		mVertexBuffers.clear();
	}

	void VertexArrayVulkan::close(VkDevice logicDevice) {
		for (std::shared_ptr<VertexBufferVulkan> vertexBuffer : mVertexBuffers) {
			vertexBuffer->close(logicDevice);
		}

		if (!mSharingIndexBuffer) {
			mIndexBuffer->close(logicDevice);
		}

		mUsingGpuResource = false;
	}

	bool VertexArrayVulkan::isUsingGpuResource() const {
		for (std::shared_ptr<VertexBufferVulkan> vertexBuffer : mVertexBuffers) {
			if (vertexBuffer->isUsingGpuResource()) {
				return true;
			}
		}

		return !mSharingIndexBuffer && mIndexBuffer->isUsingGpuResource();
	}
}
