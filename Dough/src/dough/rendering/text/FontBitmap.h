#pragma once

#include "dough/files/FntFileData.h"
#include "dough/rendering/textures/TextureVulkan.h"
#include "dough/scene/geometry/primitives/Quad.h"
#include "dough/rendering/text/ETextRenderMethod.h"

namespace DOH {

	//All values except PageId are normalised to Page dimensions
	struct GlyphData {
		glm::vec2 Offset;
		glm::vec2 Size;
		glm::vec2 TexCoordTopLeft;
		glm::vec2 TexCoordBotRight;
		float AdvanceX;
		uint32_t PageId;
	};

	//struct KerningData {
	//	uint32_t FirstGlyphId;
	//	uint32_t SecondGlyphId;
	//	float Amount;
	//};

	//TODO:: Provide hash function so this struct can be used as a key in unordererd_map
	//struct KerningMapKey {
	//	uint32_t FirstGlyphId;
	//	uint32_t SecondGlyphId;
	//};

	class FontBitmap {

	private:
		constexpr static const uint32_t TAB_SPACE_COUNT = 4;
		
		const ETextRenderMethod mTextRenderMethod;
		std::vector<std::shared_ptr<TextureVulkan>> mPageTextures;
		std::unordered_map<uint32_t, GlyphData> mGlyphMap;
		//std::unordered_map<KerningMapKey, float> mKerningMap;
		//std::vector<KerningData> mKernings;
		uint32_t mPageCount;
		float mSpaceWidthNorm;
		float mLineHeightNorm;
		float mBaseNorm;

	public:
		FontBitmap() = delete;
		FontBitmap(const FontBitmap& copy) = delete;
		FontBitmap operator=(const FontBitmap& assignment) = delete;

		FontBitmap(const char* filepath, const char* imageDir, ETextRenderMethod textRenderMethod);

		inline const float getSpaceWidthNorm() const { return mSpaceWidthNorm; }
		inline const float getTabWidthNorm() const { return mSpaceWidthNorm * static_cast<float>(FontBitmap::TAB_SPACE_COUNT); }
		inline const float getLineHeightNorm() const { return mLineHeightNorm; }
		inline const float getBaseNorm() const { return mBaseNorm; }
		inline const std::unordered_map<uint32_t, GlyphData>& getGlyphMap() const { return mGlyphMap; }
		//inline const std::unordered_map<KerningMapKey, float>& getKerningMap() const { return mKerningMap; }
		//inline const std::vector<KerningData>& getKernings() const { return mKernings; }
		inline const std::shared_ptr<TextureVulkan> getPageTexture(const uint32_t pageId) const { return mPageTextures[pageId]; }
		inline const std::vector<std::shared_ptr<TextureVulkan>>& getPageTextures() const { return mPageTextures; }
		inline const uint32_t getPageCount() const { return mPageCount; }
		inline const ETextRenderMethod getTextRenderMethod() const { return mTextRenderMethod; }
	};
}
