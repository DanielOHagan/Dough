#include "dough/rendering/textures/TextureAtlas.h"

#include "dough/Logging.h"
#include "dough/files/ResourceHandler.h"
#include "dough/files/IndexedAtlasInfoFileData.h"

#include <tracy/public/tracy/Tracy.hpp>

namespace DOH {

	MonoSpaceTextureAtlas::MonoSpaceTextureAtlas(
		VkDevice logicDevice,
		VkPhysicalDevice physicalDevice,
		const std::string& textureFilePath,
		const uint32_t rowCount,
		const uint32_t colCount
	) : TextureVulkan(logicDevice, physicalDevice, textureFilePath),
		mRowCount(rowCount != 0 ? rowCount : 1),
		mColCount(colCount != 0 ? colCount : 1),
		mNormalisedInnerTextureWidth(1.0f / mRowCount),
		mNormalisedInnerTextureHeight(1.0f / mColCount)
	{}

	std::array<float, 4> MonoSpaceTextureAtlas::getInnerTextureCoords(const uint32_t row, const uint32_t col) const {
		//Row & Col start from the top left of the texture.

		const glm::vec2 origin = getInnerTextureCoordsOrigin(row, col);

		std::array<float, 4> arr = {
			//Bottom Left
			origin.x,
			origin.y + mNormalisedInnerTextureHeight,

			//Top Right
			origin.x + mNormalisedInnerTextureWidth,
			origin.y
		};

		return arr;
	}

	std::array<float, 4> MonoSpaceTextureAtlas::getInnerTextureCoords(
		const uint32_t rowTop,
		const uint32_t rowBot,
		const uint32_t colLeft,
		const uint32_t colRight
	) const {
		const glm::vec2 origin = getInnerTextureCoordsOrigin(rowTop, colLeft);

		std::array<float, 4> arr = {
			//Bottom Left
			origin.x,
			origin.y + (rowBot * mNormalisedInnerTextureHeight),

			//Top Right
			origin.x + (colRight * mNormalisedInnerTextureWidth),
			origin.y
		};

		return arr;
	}

	IndexedTextureAtlas::IndexedTextureAtlas(
		VkDevice logicDevice,
		VkPhysicalDevice physicalDevice,
		const char* atlasInfoFilePath,
		const char* atlasTextureDir
	) : TextureVulkan() {
		ZoneScoped;

		std::shared_ptr<IndexedAtlasInfoFileData> atlasFileData = ResourceHandler::loadIndexedTextureAtlas(atlasInfoFilePath);

		if (atlasFileData == nullptr) {
			//TODO:: Handle this OUTSIDE of this function, maybe have mName = "FAILED" or mChannels = INT_MAX to signal that the texture creation failed.
			LOG_ERR("Failed to loadIndexedTextureAtlas: " << atlasInfoFilePath);
			return;
		}

		mName = atlasFileData->Name;
		mInnerTextureMap = atlasFileData->InnerTextures;
		mAnimations = atlasFileData->Animations;

		std::string imageFilePath = atlasTextureDir;
		imageFilePath.append(atlasFileData->TextureFileName);

		TextureCreationData textureCreationData = ResourceHandler::loadTexture(imageFilePath.c_str());
		if (textureCreationData.Failed) {
			//TODO:: Handle this OUTSIDE of this function, maybe have mName = "FAILED" or mChannels = INT_MAX to signal that the texture creation failed.
			LOG_ERR("IndexedAtlas " << atlasFileData->Name << " failed to loadTexture: " << imageFilePath);
			return;
		}

		mWidth = textureCreationData.Width;
		mHeight = textureCreationData.Height;
		mChannels = textureCreationData.Channels;

		//IMPORTANT:: Textures used in the engine are assumed to have 4 channels when used.
		VkDeviceSize imageSize = textureCreationData.Width * textureCreationData.Height * 4;

		load(textureCreationData.Data, imageSize);

		mId = ResourceHandler::getNextUniqueTextureId();
	}
}
