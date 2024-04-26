#pragma once

#include "dough/rendering/renderables/IRenderable.h"
#include "dough/rendering/ModelVulkan.h"

namespace DOH {

	struct TransformationData {
		glm::mat4x4 Translation = glm::mat4x4(1.0f);
		glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
		float Scale = 1.0f;

		//Initialise translation to identity and not using pos, rotation, scale,
		// because the matrix may not be used.
		TransformationData() = default;
		TransformationData(
			glm::vec3 pos,
			glm::vec3 rotationDegs,
			float scale
		) : Translation(glm::mat4x4(1.0f)),
			Position(pos),
			Rotation(rotationDegs),
			Scale(scale)
		{}

		void updateTranslationMatrix() {
			Translation = glm::translate(
				glm::mat4x4(1.0f),
				{ Position[0], Position[1], Position[2] }
			);

			Translation *= glm::eulerAngleYXZ(
				glm::radians(Rotation[0]),
				glm::radians(Rotation[1]),
				glm::radians(Rotation[2])
			);

			Translation = glm::scale(
				Translation,
				{ Scale, Scale, Scale }
			);
		}
	};

	class RenderableModelVulkan : public IRenderable {
	public:
		//TODO:: is a name necessary?
		std::string Name;
		//TODO:: does this have to be a shared_ptr ?
		std::shared_ptr<ModelVulkan> Model;
		std::shared_ptr<TransformationData> Transformation;
		bool Render;
		bool RenderWireframe;

		RenderableModelVulkan(
			const std::string& name,
			std::shared_ptr<ModelVulkan> model,
			std::shared_ptr<TransformationData> transformation
		) : Name(name),
			Model(model),
			Transformation(transformation),
			Render(true),
			RenderWireframe(false)
		{}

		RenderableModelVulkan(
			const std::string& name,
			std::shared_ptr<ModelVulkan> model
		) : Name(name),
			Model(model),
			Transformation({}),
			Render(true),
			RenderWireframe(false)
		{}

		virtual VertexArrayVulkan& getVao() const override { return Model->getVao(); };
		//NOTE::Assumes pipeline wants transformation data for push constant
		virtual void* getPushConstantPtr() const override { return &Transformation->Translation; };
		virtual bool isIndexed() const override { return true; }

		inline std::shared_ptr<ModelVulkan> getModel() const { return Model; }
		inline const std::string& getName() const { return Name; }
		inline void setName(const std::string& name) { Name = name; }
	};
}
