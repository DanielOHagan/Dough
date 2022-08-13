#include "dough/rendering/textures/TextureAtlasVulkan.h"

#include "dough/Logging.h"

namespace DOH {

	MonoSpaceTextureAtlasVulkan::MonoSpaceTextureAtlasVulkan(
		VkDevice logicDevice,
		VkPhysicalDevice physicalDevice,
		const std::string& filePath,
		const uint32_t rowCount,
		const uint32_t colCount
	) : TextureVulkan(logicDevice, physicalDevice, filePath),
		mRowCount(rowCount != 0 ? rowCount : 1),
		mColCount(colCount != 0 ? colCount : 1),
		mNormalisedInnerTextureWidth(1.0f / mRowCount),
		mNormalisedInnerTextureHeight(1.0f / mColCount)
	{}

	glm::vec2 MonoSpaceTextureAtlasVulkan::getInnerTextureCoordsOrigin(const uint32_t row, const uint32_t col) const {
		return { row * mNormalisedInnerTextureWidth, col * mNormalisedInnerTextureHeight };
	}

	std::array<float, 8> MonoSpaceTextureAtlasVulkan::getInnerTextureCoords(const uint32_t row, const uint32_t col) const {
		const glm::vec2 origin = getInnerTextureCoordsOrigin(row, col);

		std::array<float, 8> arr = {
			origin.x,
			origin.y + mNormalisedInnerTextureHeight,
		
			origin.x + mNormalisedInnerTextureWidth,
			origin.y + mNormalisedInnerTextureHeight,
		
			origin.x + mNormalisedInnerTextureWidth,
			origin.y,
		
			origin.x,
			origin.y
		};

		return arr;
	}

	std::array<float, 8> MonoSpaceTextureAtlasVulkan::getInnerTextureCoords(
		const uint32_t rowTop,
		const uint32_t rowBot,
		const uint32_t colLeft,
		const uint32_t colRight
	) const {
		const glm::vec2 origin = getInnerTextureCoordsOrigin(rowTop, colLeft);

		std::array<float, 8> arr = {
			origin.x,
			origin.y + (rowBot * mNormalisedInnerTextureHeight),

			origin.x + (colRight * mNormalisedInnerTextureWidth),
			origin.y + (rowBot * mNormalisedInnerTextureHeight),

			origin.x + (colRight * mNormalisedInnerTextureWidth),
			origin.y,

			origin.x,
			origin.y
		};

		return arr;
	}
}
