#pragma once

#include "dough/scene/geometry/AGeometry.h"
#include "dough/rendering/TextureVulkan.h"

#include <optional>

namespace DOH {

	class Quad : public AGeometry {

	private:
		//TODO:: Material member to handle texture or solid/pattern/gradient/vertex colour
		std::optional<std::reference_wrapper<TextureVulkan>> mTexture;
		//TextureVulkan& mTexture;
		//std::array<float, 8> mTextureCoords; //Interlaced ordering


	public:
		Quad()
		:	AGeometry({ 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f }, 0.0f),
			Colour(1.0f, 0.0f, 1.0f, 1.0f)
		{}
		Quad(
			glm::vec3 pos,
			glm::vec2 size,
			glm::vec4 colour,
			float rotationRads = 0.0f,
			std::shared_ptr<TextureVulkan> texture = nullptr
		) : AGeometry(pos, size, rotationRads),
			Colour(colour),
			mTexture(*texture)
		{}

		glm::vec4 Colour;

		inline void setColour(glm::vec4 colour) { Colour = colour; }
		inline void setColourRGB(float r, float g, float b) { Colour.x = r; Colour.y = g; Colour.z = b; }
		inline void setColourRGBA(float r, float g, float b, float a) { Colour.x = r; Colour.y = g; Colour.z = b; Colour.w = a; }
		inline bool hasTexture() const { return mTexture.has_value(); }
		inline TextureVulkan& getTexture() const { return mTexture.value(); }
	};
}
