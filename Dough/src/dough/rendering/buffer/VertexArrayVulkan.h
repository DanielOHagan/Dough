#pragma once

#include "dough/rendering/buffer/VertexBufferVulkan.h"
#include "dough/rendering/buffer/IndexBufferVulkan.h"

namespace DOH {

	class VertexArrayVulkan {

	private:
		std::vector<std::shared_ptr<VertexBufferVulkan>> mVertexBuffers;
		std::shared_ptr<IndexBufferVulkan> mIndexBuffer;

	public:
		VertexArrayVulkan();

		void bind(VkCommandBuffer cmdBuffer);

		void addVertexBuffer(std::shared_ptr<VertexBufferVulkan> vertexBuffer);
		void close(VkDevice logicDevice);

		inline void setIndexBuffer(std::shared_ptr<IndexBufferVulkan> indexBuffer) { mIndexBuffer = indexBuffer; }
		inline IndexBufferVulkan& getIndexBuffer() const { return *mIndexBuffer.get(); }
		inline std::vector<std::shared_ptr<VertexBufferVulkan>>& getVertexBuffers() { return mVertexBuffers; }

	};
}
