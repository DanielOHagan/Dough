#include "dough/rendering/ModelVulkan.h"

#include "dough/application/Application.h"

#include <tracy/public/tracy/Tracy.hpp>

namespace DOH {

	ModelVulkan::ModelVulkan(std::shared_ptr<Model3dCreationData> modelCreationData) {
		ZoneScoped;

		auto& context = Application::get().getRenderer().getContext();
		std::shared_ptr<VertexBufferVulkan> vbo = context.createStagedVertexBuffer(
			modelCreationData->VertexInputLayout,
			modelCreationData->Vertices.data(),
			modelCreationData->Vertices.size() * sizeof(float),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		std::shared_ptr<IndexBufferVulkan> ibo = context.createStagedIndexBuffer(
			modelCreationData->Indices.data(),
			modelCreationData->Indices.size() * sizeof(uint32_t)
		);

		mVao = context.createVertexArray();
		mVao->addVertexBuffer(vbo);
		mVao->setIndexBuffer(ibo);
		mVao->setDrawCount(static_cast<uint32_t>(modelCreationData->Indices.size()));

		mUsingGpuResource = true;
	}

	ModelVulkan::~ModelVulkan() {
		if (isUsingGpuResource()) {
			//Model is mainly a syntactic veil for a VAO. The VAO destructor will also run an isUsingGpuResource() check.
			LOG_ERR("Model GPU resource NOT released before destructor was called.");

			//NOTE:: This is to stop the IGPUResource::~IGPUReource from logging a misleading error message.
			mUsingGpuResource = false;
		}
	}

	void ModelVulkan::close(VkDevice logicDevice) {
		ZoneScoped;

		mVao->close(logicDevice);
		mUsingGpuResource = false;
	}

	std::shared_ptr<ModelVulkan> ModelVulkan::createModel(const std::string& filepath, const AVertexInputLayout& vertexInputLayout) {
		return std::make_shared<ModelVulkan>(ResourceHandler::loadObjModel(filepath, vertexInputLayout));
	}
}
