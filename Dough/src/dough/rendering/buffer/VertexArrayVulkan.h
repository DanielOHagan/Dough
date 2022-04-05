#pragma once

#include "dough/rendering/buffer/VertexBufferVulkan.h"
#include "dough/rendering/buffer/IndexBufferVulkan.h"

namespace DOH {

	class VertexArrayVulkan : public IGPUResourceVulkan {

		//TODO:: Multiple VBOs aren't fully supported, for now only use one
		//	It might not be worth adding support for > 1 VBOs as interweaving data
		//	is faster than sorting many different vertex input possibilities

	private:

		static const uint32_t MAX_VBO_COUNT;

		std::vector<std::shared_ptr<VertexBufferVulkan>> mVertexBuffers;
		std::shared_ptr<IndexBufferVulkan> mIndexBuffer;

	public:
		VertexArrayVulkan() = default;

		VertexArrayVulkan(const VertexArrayVulkan& copy) = delete;
		VertexArrayVulkan operator=(const VertexArrayVulkan& assignment) = delete;

		virtual bool isUsingGpuResource() const override;

		void bind(VkCommandBuffer cmdBuffer);

		void addVertexBuffer(std::shared_ptr<VertexBufferVulkan> vertexBuffer);
		virtual void close(VkDevice logicDevice) override;

		inline void setIndexBuffer(std::shared_ptr<IndexBufferVulkan> indexBuffer) { mIndexBuffer = indexBuffer; }
		inline IndexBufferVulkan& getIndexBuffer() const { return *mIndexBuffer; }
		inline std::vector<std::shared_ptr<VertexBufferVulkan>>& getVertexBuffers() { return mVertexBuffers; }
	};
}
