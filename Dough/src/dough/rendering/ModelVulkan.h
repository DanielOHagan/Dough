#pragma once

#include "dough/Core.h"
#include "dough/rendering/IGPUResourceVulkan.h"
#include "dough/ResourceHandler.h"

namespace DOH {

	class VertexArrayVulkan;

	class ModelVulkan : public IGPUResourceVulkan {

	private:
		std::shared_ptr<VertexArrayVulkan> mVao;

		ModelVulkan() = delete;
		ModelVulkan(const ModelVulkan& copy) = delete;
		ModelVulkan operator=(const ModelVulkan& assignment) = delete;

	public:
		ModelVulkan(std::shared_ptr<Model3dCreationData> modelCreationData);

		inline VertexArrayVulkan& getVao() const { return *mVao; }

		virtual void close(VkDevice logicDevice) override;

		static std::shared_ptr<ModelVulkan> createModel(const std::string& filepath);
	};
}
