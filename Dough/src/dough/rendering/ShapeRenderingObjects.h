#pragma once

#include "dough/rendering/RenderingContextVulkan.h"
#include "dough/scene/geometry/AGeometry.h"

#include <memory>

namespace DOH {

	static std::array<const char*, 4> EShapeStrings = {
		"NONE",

		"CIRCLE",
		"TRIANGLE",
		"QUAD"
	};
	enum class EShape {
		NONE = 0,

		CIRCLE,
		TRIANGLE,
		QUAD
	};

	template<typename T, typename = std::enable_if<std::is_base_of<ARenderBatch<AGeometry>, T>::value>>
	class ShapeRenderingObjects : public IGPUResourceOwnerVulkan {
	public:
		ShapeRenderingObjects()
		:	Shape(EShape::NONE)
		{}
		ShapeRenderingObjects(EShape shape)
		:	Shape(shape)
		{}

		//TODO:: Some kind of BatchManager<T> for control and functionality (e.g. allowing for a limit of how many batches, having a batch that never changes, etc...)
		std::vector<std::shared_ptr<T>> GeoBatches;
		std::vector<std::shared_ptr<SimpleRenderable>> Renderables;
		std::shared_ptr<ShaderProgram> Program;
		std::shared_ptr<ShaderVulkan> VertexShader;
		std::shared_ptr<ShaderVulkan> FragmentShader;
		std::shared_ptr<GraphicsPipelineVulkan> Pipeline;
		std::shared_ptr<DescriptorSetsInstanceVulkan> DescriptorSetsInstance;
		std::unique_ptr<GraphicsPipelineInstanceInfo> PipelineInstanceInfo;
		EShape Shape;

		virtual void addOwnedResourcesToClose(RenderingContextVulkan& context) override {
			Program->addOwnedResourcesToClose(context);
			context.addGpuResourceToClose(VertexShader);
			context.addGpuResourceToClose(FragmentShader);
			context.addGpuResourceToClose(Pipeline);
			for (auto& renderable : Renderables) {
				context.addGpuResourceToClose(renderable->getVaoPtr());
			}
		}

		//IMPORTANT:: Renderables count should match this
		inline uint32_t getBatchCount() const { return static_cast<uint32_t>(GeoBatches.size()); }
	};
}
