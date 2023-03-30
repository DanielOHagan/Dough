#pragma once

#include "dough/Core.h"
#include "dough/rendering/IGPUResourceVulkan.h"
#include "dough/files/ResourceHandler.h"

namespace DOH {

	class VertexArrayVulkan;

	//NOTE:: ModelVulkan has ownership over the VAO created by createModel()
	//	While ModelVulkan itself doesn't have any direct GPU resources by inheriting IGPUResourceVulkan
	//	this adds extra information for debugging GPU resource closing.
	class ModelVulkan : public IGPUResourceVulkan {
	private:
		std::shared_ptr<VertexArrayVulkan> mVao;

		ModelVulkan() = delete;
		ModelVulkan(const ModelVulkan& copy) = delete;
		ModelVulkan operator=(const ModelVulkan& assignment) = delete;

	public:
		ModelVulkan(std::shared_ptr<Model3dCreationData> modelCreationData);

		virtual ~ModelVulkan() override;
		virtual void close(VkDevice logicDevice) override;

		inline VertexArrayVulkan& getVao() const { return *mVao; }

		static std::shared_ptr<ModelVulkan> createModel(const std::string& filepath, const EVertexType vertextype);
	};
}
