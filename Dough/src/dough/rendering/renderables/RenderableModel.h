#pragma once

#include "dough/rendering/renderables/IRenderable.h"
#include "dough/rendering/ModelVulkan.h"

namespace DOH {

	struct TransformationData {
		glm::mat4x4 Translation = glm::mat4x4(1.0f);
		float Position[3] = { 0.0f, 0.0f, 0.0f };
		float Rotation[3] = { 0.0f, 0.0f, 0.0f };
		float Scale = 1.0f;
	};

	class RenderableModelVulkan : public IRenderable {
	
	public:
		std::string Name;
		std::shared_ptr<ModelVulkan> Model;
		std::shared_ptr<TransformationData> Transformation;
		bool ShouldRender;

		RenderableModelVulkan(
			const std::string& name,
			std::shared_ptr<ModelVulkan> model,
			std::shared_ptr<TransformationData> transformation
		) : Name(name),
			Model(model),
			Transformation(transformation),
			ShouldRender(true)
		{}

		virtual VertexArrayVulkan& getVao() const override { return Model->getVao(); };
		//NOTE::Assumes pipeline wants transformation data for push constant
		virtual void* getPushConstantPtr() const override { return &Transformation->Translation; };

		inline std::shared_ptr<ModelVulkan> getModel() const { return Model; }
		inline const std::string& getName() const { return Name; }
		inline void setName(const std::string& name) { Name = name; }
	};

}
