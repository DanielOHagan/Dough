#pragma once

#include "dough/scene/geometry/AGeometry2D.h"
#include "dough/rendering/TextureVulkan.h"

#include <optional>

namespace DOH {

	class Quad : public AGeometry2D {

	private:
		//TODO:: Material member to handle texture or solid/pattern/gradient/vertex colour
		std::unique_ptr<glm::vec4> mColour;
		std::optional<TextureVulkan&> mOptionalTexture;

	public:
		Quad(
			glm::vec3& pos,
			glm::vec2& size,
			glm::vec4& colour,
			std::shared_ptr<TextureVulkan> texture,
			float rotationRads = 0.0f
		) :	AGeometry2D(pos, size, rotationRads),
			mColour(std::make_unique<glm::vec4>(colour)),
			mOptionalTexture({})
		{
			if (texture != nullptr) {
				mOptionalTexture.emplace(texture);
			}
		}

		inline glm::vec4& getColour() const { return *mColour; }
		inline void setColour(glm::vec4& colour) { mColour->x = colour.x; mColour->y = colour.y; mColour->z = colour.z; mColour->w = colour.w; }
		inline void setColour(float r, float g, float b, float a) { mColour->x = r; mColour->y = g; mColour->z = b; mColour->w = a; }
		inline bool hasTexture() const { return mOptionalTexture.has_value(); }
		inline std::optional<TextureVulkan> getTexture() const { mOptionalTexture.value(); }
		inline void setTexture(TextureVulkan& texture) { mOptionalTexture.emplace(texture); }
	};
}
