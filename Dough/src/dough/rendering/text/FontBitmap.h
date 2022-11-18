#pragma once

#include "dough/model/FntFileData.h"
#include "dough/rendering/textures/TextureVulkan.h"
#include "dough/scene/geometry/Quad.h"

namespace DOH {

	class FontBitmap {

	private:
		//TODO:: currently only supports .fnt files
		//TEMP:: assume page count is 1
		std::vector<std::shared_ptr<TextureVulkan>> mPageTextures;
		std::shared_ptr<FntFileData> mFntFileData;

		//uint32_t for inner texture ID, with bottom-left ?normalised? texture coord - texture size
		// TODO:: Calculate texture quads for chars on initialisation and release file data?
		//std::unordered_map<uint32_t, std::tuple<glm::vec2>> mInnerTextureMap;

		uint32_t mSpaceWidthPx;

	public:
		FontBitmap() = delete;
		FontBitmap(const FontBitmap& copy) = delete;
		FontBitmap operator=(const FontBitmap& assignment) = delete;

		FontBitmap(const char* filepath, const char* imageDir);

		inline const uint32_t getSpaceWidthPx() const { return mSpaceWidthPx; };
		inline void setSpaceWidthPx(const uint32_t spaceWidthPx) { mSpaceWidthPx = spaceWidthPx; }
		inline const FntFileData& getFileData() const { return *mFntFileData; }
		inline const std::shared_ptr<TextureVulkan> getPageTexture(const uint32_t pageId) const { return mPageTextures[pageId < mFntFileData->PageCount ? pageId : 0]; }
		inline const std::vector<std::shared_ptr<TextureVulkan>>& getPageTextures() const { return mPageTextures; }
		inline const uint32_t getPageCount() const { return mFntFileData->PageCount; }
	};
}
