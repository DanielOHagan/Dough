#pragma once

#include "dough/Core.h"
#include "dough/rendering/textures/TextureVulkan.h"
#include "dough/Maths.h"

#include <unordered_map>

namespace DOH {

	/**
	* Description of texture within a texture atlas.
	*/
	struct InnerTexture {
		//Texel coords of inner texture, origin at Top-Left, store order is Top-Left, Top-Right, Bottom-Right, Bottom-Left.
		std::array<uint32_t, 8> TexelCoords;
		std::array<float, 8> TextureCoords;
		//uint32 TileWidth; //The number of tiles along the X-axis this inner texture uses.
		//uint32 TileHeight; //The number of tiles along the Y-aixs this inner texture uses.

		inline std::array<uint32_t, 2> getTopLeftTexels() const { return { TexelCoords[0], TexelCoords[1] }; }
		inline std::array<uint32_t, 2> getTopRightTexels() const { return { TexelCoords[2], TexelCoords[3] }; }
		inline std::array<uint32_t, 2> getBottomRightTexels() const { return { TexelCoords[4], TexelCoords[5] }; }
		inline std::array<uint32_t, 2> getBottomLeftTexels() const { return { TexelCoords[6], TexelCoords[7] }; }
		inline std::array<float, 2> getTopLeftTexCoords() const { return { TextureCoords[0], TextureCoords[1] }; }
		inline std::array<float, 2> getTopRightTexCoords() const { return { TextureCoords[2], TextureCoords[3] }; }
		inline std::array<float, 2> getBottomRightTexCoords() const { return { TextureCoords[4], TextureCoords[5] }; }
		inline std::array<float, 2> getBottomLeftTexCoords() const { return { TextureCoords[6], TextureCoords[7] }; }
	};

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
		std::array<float, 8> getInnerTextureCoords(const uint32_t row, const uint32_t col) const;
		std::array<float, 8> getInnerTextureCoords(const uint32_t rowTop, const uint32_t rowBot, const uint32_t colLeft, const uint32_t colRight) const;
		const uint32_t getRowCount() const { return mRowCount; }
		const uint32_t getColCount() const { return mColCount; }
	};

	//Does not inherit from texture because texture requires 
	class IndexedTextureAtlas : public TextureVulkan {
	private:
		//Key: Name Value: TexCoords
		std::unordered_map<std::string, InnerTexture> mInnerTextureMap;

	public:
		IndexedTextureAtlas(VkDevice logicDevice, VkPhysicalDevice physicalDevice, const char* atlasInfoFilePath, const char* atlasTextureDir);

		inline const std::unordered_map<std::string, InnerTexture>& getInnerTextures() const { return mInnerTextureMap; }
		//std::optional<InnerTexture> getInnerTexture(const char* innerTextureName) const;
		const InnerTexture& getInnerTexture(const char* innerTextureName) const { return mInnerTextureMap.find(innerTextureName)->second; }
	};
}
