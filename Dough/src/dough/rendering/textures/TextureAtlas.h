#pragma once

#include "dough/Core.h"
#include "dough/rendering/textures/TextureVulkan.h"
#include "dough/Maths.h"
#include "dough/rendering/animations/TextureAtlasAnimation.h"

#include <unordered_map>

namespace DOH {
	
	/**
	* Description of texture within a texture atlas.
	*
	* TODO:: Shouldn't this just be std::array<float, 4> TextureCoords; TopLeft->BotRight. And have many functions like getLeft() { return TextureCoords[1]; } etc... And Texels just cast to uint.
	*/
	struct InnerTexture {
		//Texel coords of inner texture, origin at Top-Left, store order is Top-Left, Top-Right, Bottom-Right, Bottom-Left.
		std::array<uint32_t, 8> TexelCoords;
		std::array<float, 8> TextureCoords;
		//uint32 TileWidth; //The number of tiles along the X-axis this inner texture uses.
		//uint32 TileHeight; //The number of tiles along the Y-aixs this inner texture uses.

		//Useful getters for quad shaped textures
		inline std::array<uint32_t, 2> getTopLeftTexels() const { return { TexelCoords[0], TexelCoords[1] }; }
		inline std::array<uint32_t, 2> getTopRightTexels() const { return { TexelCoords[2], TexelCoords[3] }; }
		inline std::array<uint32_t, 2> getBottomRightTexels() const { return { TexelCoords[4], TexelCoords[5] }; }
		inline std::array<uint32_t, 2> getBottomLeftTexels() const { return { TexelCoords[6], TexelCoords[7] }; }
		inline std::array<float, 2> getTopLeftTexCoords() const { return { TextureCoords[0], TextureCoords[1] }; }
		inline std::array<float, 2> getTopRightTexCoords() const { return { TextureCoords[2], TextureCoords[3] }; }
		inline std::array<float, 2> getBottomRightTexCoords() const { return { TextureCoords[4], TextureCoords[5] }; }
		inline std::array<float, 2> getBottomLeftTexCoords() const { return { TextureCoords[6], TextureCoords[7] }; }
		//Return array of 4 floats of the texture coords, ordered botLeft.x, botLeft.y, topRight.x, topRight.y to match quad renderer input.
		//TODO:: This ordering botLeft->... is different from TextureCoords and may cause confusion.
		inline std::array<float, 4> getTexCoordsAsSquare() const { return { TextureCoords[6], TextureCoords[7], TextureCoords[2], TextureCoords[3] }; }


		inline uint32_t getWidthTexels() const { return TexelCoords[2] - TexelCoords[0]; } //Any ___Right.x value - and ___Left.x value
		inline uint32_t getHeightTexels() const { return TexelCoords[5] - TexelCoords[1]; } //Any ___Bottom.y value - and ___Top.y value
		inline float getWidthTexCoord() const { return TextureCoords[2] - TextureCoords[0]; } //Any ___Right.x value - and ___Left.x value
		inline float getHeightTexCoord() const { return TextureCoords[5] - TextureCoords[1] ; } //Any ___Bottom.y value - and ___Top.y value
	};

	//TODO:: Maybe an InnerTextureGroup which inherits from InnerTexture.
	/*
	e.g. 
	struct InnerTextureGroup : public InnerTexture {
		uint32 TileWidth; //The number of tiles along the X-axis this inner texture uses.
		uint32 TileHeight; //The number of tiles along the Y-aixs this inner texture uses.

		//The TexelCoords & TextureCoords being the bounding box of the inner textures
	}
	*/

	class MonoSpaceTextureAtlas : public TextureVulkan {
	private:
		const uint32_t mRowCount;
		const uint32_t mColCount;
		const float mNormalisedInnerTextureWidth;
		const float mNormalisedInnerTextureHeight;

	public:
		MonoSpaceTextureAtlas() = delete;
		MonoSpaceTextureAtlas(const MonoSpaceTextureAtlas& copy) = delete;
		MonoSpaceTextureAtlas operator=(const MonoSpaceTextureAtlas& assignment) = delete;

		MonoSpaceTextureAtlas(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			const std::string& textureFilePath,
			const uint32_t rowCount,
			const uint32_t colCount
		);

		//Row & columns range from 0 -> (row or column count - 1).
		inline glm::vec2 getInnerTextureCoordsOrigin(const uint32_t row, const uint32_t col) const {
			return { row * mNormalisedInnerTextureWidth, col * mNormalisedInnerTextureHeight };
		}
		std::array<float, 4> getInnerTextureCoords(const uint32_t row, const uint32_t col) const;
		std::array<float, 4> getInnerTextureCoords(const uint32_t rowTop, const uint32_t rowBot, const uint32_t colLeft, const uint32_t colRight) const;
		const uint32_t getRowCount() const { return mRowCount; }
		const uint32_t getColCount() const { return mColCount; }
	};

	//Does not inherit from texture because texture requires 
	class IndexedTextureAtlas : public TextureVulkan {
	private:
		//Key: Name Value: TexCoords
		std::unordered_map<std::string, InnerTexture> mInnerTextureMap;
		std::unordered_map<std::string, TextureAtlasAnimation> mAnimations;

	public:
		IndexedTextureAtlas(VkDevice logicDevice, VkPhysicalDevice physicalDevice, const char* atlasInfoFilePath, const char* atlasTextureDir);

		inline const std::unordered_map<std::string, InnerTexture>& getInnerTextures() const { return mInnerTextureMap; }
		inline const std::unordered_map<std::string, TextureAtlasAnimation>& getAnimations() const { return mAnimations; }
		
		const InnerTexture& getInnerTexture(const char* innerTextureName) const { return mInnerTextureMap.find(innerTextureName)->second; }
		const TextureAtlasAnimation& getAnimation(const char* animationName) const { return mAnimations.find(animationName)->second; }
	};
}
