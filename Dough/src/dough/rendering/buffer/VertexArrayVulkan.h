#pragma once

#include "dough/rendering/buffer/VertexBufferVulkan.h"
#include "dough/rendering/buffer/IndexBufferVulkan.h"

namespace DOH {

	class VertexArrayVulkan : public IGPUResourceVulkan {

	private:
		std::vector<std::shared_ptr<VertexBufferVulkan>> mVertexBuffers;
		std::shared_ptr<IndexBufferVulkan> mIndexBuffer;

	public:
		VertexArrayVulkan();

		VertexArrayVulkan(const VertexArrayVulkan& copy) = delete;
		VertexArrayVulkan operator=(const VertexArrayVulkan& assignment) = delete;

		void bind(VkCommandBuffer cmdBuffer);

		void addVertexBuffer(std::shared_ptr<VertexBufferVulkan> vertexBuffer);
		virtual void close(VkDevice logicDevice) override;

		inline void setIndexBuffer(std::shared_ptr<IndexBufferVulkan> indexBuffer) { mIndexBuffer = indexBuffer; }
		inline IndexBufferVulkan& getIndexBuffer() const { return *mIndexBuffer; }
		inline std::vector<std::shared_ptr<VertexBufferVulkan>>& getVertexBuffers() { return mVertexBuffers; }
	};
}
