#include "dough/rendering/ModelVulkan.h"

#include "dough/rendering/ObjInit.h"

namespace DOH {

	ModelVulkan::ModelVulkan(const Model3dCreationData& modelCreationData) {
		std::shared_ptr<VertexBufferVulkan> vbo = ObjInit::stagedVertexBuffer(
			modelCreationData.BufferElements,
			modelCreationData.Vertices.data(),
			modelCreationData.VertexBufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		std::shared_ptr<IndexBufferVulkan> ibo = ObjInit::stagedIndexBuffer(
			modelCreationData.Indices.data(),
			modelCreationData.IndexBufferSize
		);

		mVao = ObjInit::vertexArray();
		mVao->addVertexBuffer(vbo);
		mVao->setIndexBuffer(ibo);
		mVao->setDrawCount(static_cast<uint32_t>(modelCreationData.Indices.size()));

		mUsingGpuResource = true;
	}

	void ModelVulkan::close(VkDevice logicDevice) {
		mVao->close(logicDevice);

		mUsingGpuResource = false;
	}

	std::shared_ptr<ModelVulkan> ModelVulkan::createModel(const std::string& filepath) {
		return std::make_shared<ModelVulkan>(ResourceHandler::loadObjModel(filepath));
	}
}