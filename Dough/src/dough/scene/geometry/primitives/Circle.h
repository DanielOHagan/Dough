#pragma once

#include "dough/scene/geometry/AGeometry.h"

#include <optional>
#include <array>

namespace DOH {

	class TextureVulkan;

	class Circle : public AGeometry {
	private:
		std::optional<std::reference_wrapper<TextureVulkan>> Texture;

	public:
		//Interlaced texture coords for a quad ordered: botLeft.x, botLeft.y, topRight.x, topRight.Y
		constexpr static std::array<float, 4> DEFAULT_TEXTURE_COORDS = { 0.0f, 1.0f, 1.0f, 0.0 };
		constexpr static uint32_t BYTE_SIZE = 224u; //VertexCircle3d::BYTE_SIZE * 4u;

		glm::vec4 Colour;
		//Interlaced texture coords for a quad ordered: botLeft.x, botLeft.y, topRight.x, topRight.Y
		std::array<float, 4> TextureCoords;
		glm::vec2 Decorations;

		Circle()
		:	AGeometry({ 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f }, 0.0f),
			Colour(1.0f, 1.0f, 1.0f, 1.0f),
			TextureCoords(Circle::DEFAULT_TEXTURE_COORDS),
			Decorations(1.0f, 0.0f)
		{}
		Circle(
			glm::vec3 pos,
			glm::vec3 size,
			glm::vec4 colour,
			float rotationRads = 0.0f,
			std::shared_ptr<TextureVulkan> texture = nullptr,
			std::array<float, 4> texCoords = Circle::DEFAULT_TEXTURE_COORDS,
			float thickness = 1.0f
		) : AGeometry(pos, size, rotationRads),
			Texture(std::nullopt),
			Colour(colour),
			TextureCoords(texCoords),
			Decorations(thickness, 0.0f)
		{
			if (texture != nullptr) {
				Texture.emplace(*texture);
			}
		}

		inline void setColour(glm::vec4 colour) { Colour = colour; }
		inline void setColourRGB(float r, float g, float b) { Colour = { r, g, b, Colour.z }; }
		inline void setColourRGBA(float r, float g, float b, float a) { Colour = { r, g, b, a }; }
		inline bool hasTexture() const { return Texture.has_value(); }
		inline TextureVulkan& getTexture() const { return Texture.value(); }
		inline void setTexture(std::reference_wrapper<TextureVulkan> texture) { Texture.emplace(texture); }

		inline float getTextureCoordsBotLeftX() const { return TextureCoords[0]; };
		inline float getTextureCoordsBotLeftY() const { return TextureCoords[1]; };
		inline float getTextureCoordsTopRightX() const { return TextureCoords[2]; };
		inline float getTextureCoordsTopRightY() const { return TextureCoords[3]; };

		inline float getThickness() const { return Decorations.x; }
		inline float getFade() const { return Decorations.y; }
	};
}
