#pragma once

#include "dough/scene/geometry/AGeometry.h"
#include "dough/rendering/textures/TextureVulkan.h"

#include <optional>
#include <array>

namespace DOH {

	class Quad : public AGeometry {

	private:
		//TODO:: Material member to handle texture or solid/pattern/gradient/vertex colour
		//	I think it best if the end result is a "GameObject" class which contains Quad for purely spacial properties
		//	& a material class for Colour, Texture, etc...
		std::optional<std::reference_wrapper<TextureVulkan>> Texture;

	public:

		Quad()
		:	AGeometry({ 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f }, 0.0f),
			Colour(1.0f, 0.0f, 1.0f, 1.0f),
			TextureCoords({ 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f })
		{}
		Quad(
			glm::vec3 pos,
			glm::vec2 size,
			glm::vec4 colour,
			float rotationRads = 0.0f,
			std::shared_ptr<TextureVulkan> texture = nullptr,
			std::array<float, 8> texCoords = { 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f }
		) : AGeometry(pos, size, rotationRads),
			Texture(std::nullopt),
			Colour(colour),
			TextureCoords(texCoords)
		{
			if (texture != nullptr) {
				Texture.emplace(*texture);
			}
		}

		glm::vec4 Colour;
		std::array<float, 8> TextureCoords; //Interlaced ordering

		inline void setColour(glm::vec4 colour) { Colour = colour; }
		inline void setColourRGB(float r, float g, float b) { Colour = { r, g, b, Colour.z }; }
		inline void setColourRGBA(float r, float g, float b, float a) { Colour = { r, g, b, a }; }
		inline bool hasTexture() const { return Texture.has_value(); }
		inline TextureVulkan& getTexture() const { return Texture.value(); }
		inline void setTexture(std::reference_wrapper<TextureVulkan> texture) { Texture.emplace(texture); }
	};
}
