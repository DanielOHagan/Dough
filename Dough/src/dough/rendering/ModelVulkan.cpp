#include "dough/rendering/ModelVulkan.h"

#include "dough/rendering/ObjInit.h"

namespace DOH {

	ModelVulkan::ModelVulkan(std::shared_ptr<Model3dCreationData> modelCreationData) {
		std::shared_ptr<VertexBufferVulkan> vbo = ObjInit::stagedVertexBuffer(
			modelCreationData->VertexInputLayout,
			modelCreationData->Vertices.data(),
			modelCreationData->Vertices.size() * sizeof(float),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		std::shared_ptr<IndexBufferVulkan> ibo = ObjInit::stagedIndexBuffer(
			modelCreationData->Indices.data(),
			modelCreationData->Indices.size() * sizeof(uint32_t)
		);

		mVao = ObjInit::vertexArray();
		mVao->addVertexBuffer(vbo);
		mVao->setIndexBuffer(ibo);
		mVao->setDrawCount(static_cast<uint32_t>(modelCreationData->Indices.size()));

		mUsingGpuResource = true;
	}

	ModelVulkan::~ModelVulkan() {
		if (isUsingGpuResource()) {
			//Model is mainly a syntactic veil for a VAO. The VAO destructor will also run an isUsingGpuResource() check.
			LOG_ERR("Model GPU resource NOT released before destructor was called.");
		}
	}

	void ModelVulkan::close(VkDevice logicDevice) {
		mVao->close(logicDevice);
		mUsingGpuResource = false;
	}

	std::shared_ptr<ModelVulkan> ModelVulkan::createModel(const std::string& filepath, const AVertexInputLayout& vertexInputLayout) {
		return std::make_shared<ModelVulkan>(ResourceHandler::loadObjModel(filepath, vertexInputLayout));
	}
}
