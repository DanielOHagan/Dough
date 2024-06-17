#include "dough/rendering/buffer/VertexArrayVulkan.h"

#include "dough/Logging.h"
#include "dough/application/Application.h"

namespace DOH {

	const uint32_t VertexArrayVulkan::MAX_VBO_COUNT = 1;

	VertexArrayVulkan::VertexArrayVulkan()
	:	mIndexBuffer(nullptr),
		mDrawCount(0),
		mSharingIndexBuffer(false),
		mPushConstantData(nullptr)
	{}

	VertexArrayVulkan::~VertexArrayVulkan() {
		if (isUsingGpuResource()) {
			
			LOG_ERR("Vertex Array GPU resource NOT released before destructor was called.");

			//Verbose error message extra info
			//std::vector<VkBuffer> vertexBufferHandles;
			//const size_t vertexBuffercount = mVertexBuffers.size();
			//vertexBufferHandles.reserve(vertexBuffercount);
			//for (const auto& vertexBuffer : mVertexBuffers) {
			//	vertexBufferHandles.emplace_back(vertexBuffer->getBuffer());
			//}
			//
			//LOG_ERR("Vertex Buffer Handles: ");
			//for (size_t i = 0; i < vertexBuffercount; i++) {
			//	LOG_ERR_INLINE(" " << vertexBufferHandles[i] << " ");
			//}
			//LOG_ERR((mSharingIndexBuffer ? "Shared Index Buffer Handle: " : "Index Buffer Handle: ") << mIndexBuffer->getBuffer());

			//NOTE:: This is to stop the IGPUResource::~IGPUReource from logging a misleading error message.
			mUsingGpuResource = false;
		}
	}

	void VertexArrayVulkan::bind(VkCommandBuffer cmd) {
		AppDebugInfo& debugInfo = Application::get().getDebugInfo();
		const size_t vbSize = mVertexBuffers.size();

		if (vbSize == 0) {
			LOG_WARN("Attempt to bind VAO with 0 attached VBO's");
			return;
		} else if (vbSize > 1) {
			std::vector<VkBuffer> vertexBuffers;
			std::vector<VkDeviceSize> offsets;

			vertexBuffers.resize(vbSize);
			offsets.resize(vbSize + 1);
			offsets[0] = 0;

			for (size_t i = 0; i < vbSize; i++) {
				vertexBuffers[i] = mVertexBuffers[i]->getBuffer();
				offsets[i + 1] = static_cast<VkDeviceSize>(mVertexBuffers[i]->getVertexInputLayout().getStride());
			}

			const uint32_t vbCount = static_cast<uint32_t>(vbSize);

			vkCmdBindVertexBuffers(
				cmd,
				0,
				vbCount,
				vertexBuffers.data(),
				offsets.data()
			);
			debugInfo.VertexBufferBinds += vbCount;
			debugInfo.VertexArrayBinds++;
		} else {
			VkBuffer buffer = mVertexBuffers[0]->getBuffer();
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(
				cmd,
				0,
				1,
				&buffer,
				&offset
			);
			debugInfo.VertexBufferBinds++;
			debugInfo.VertexArrayBinds++;
		}

		if (!mSharingIndexBuffer && mIndexBuffer != nullptr) {
			mIndexBuffer->bind(cmd);
			debugInfo.IndexBufferBinds++;
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
			mVertexBuffers.emplace_back(vertexBuffer);
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

		if (!mSharingIndexBuffer && mIndexBuffer != nullptr) {
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

		return !mSharingIndexBuffer && mIndexBuffer != nullptr && mIndexBuffer->isUsingGpuResource();
	}
}
