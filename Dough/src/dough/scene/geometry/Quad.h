#pragma once

#include "dough/scene/geometry/AGeometry.h"
//#include "dough/rendering/TextureVulkan.h"

namespace DOH {

	class Quad : public AGeometry {

	private:
		//TODO:: Material member to handle texture or solid/pattern/gradient/vertex colour
		glm::vec4 mColour;
		//std::optional<TextureVulkan> mOptionalTexture;
		//TextureVulkan& mTexture;
		//std::array<float, 8> mTextureCoords; //Interlaced ordering


	public:
		Quad()
		:	AGeometry({ 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f }, 0.0f),
			mColour(1.0f, 0.0f, 1.0f, 1.0f)
		{}
		Quad(
			glm::vec3 pos,
			glm::vec2 size,
			glm::vec4 colour,
			float rotationRads = 0.0f
		) : AGeometry(pos, size, rotationRads),
			mColour(colour)
		{}

		inline glm::vec4 getColour() const { return mColour; }
		inline void setColour(glm::vec4 colour) { mColour = colour; }
		inline void setColour(float r, float g, float b, float a) { mColour.x = r; mColour.y = g; mColour.z = b; mColour.w = a; }
	};
}
