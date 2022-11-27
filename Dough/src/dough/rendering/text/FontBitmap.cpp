#include "dough/rendering/text/FontBitmap.h"

#include "dough/ResourceHandler.h"
#include "dough/Logging.h"
#include "dough/scene/geometry/Quad.h"
#include "dough/rendering/ObjInit.h"

namespace DOH {

	FontBitmap::FontBitmap(const char* filepath, const char* imageDir)
	:	mPageCount(0),
		mSpaceWidthNorm(0.0f),
		mLineHeightNorm(0.0f),
		mBaseNorm(0.0f)
	{
		std::shared_ptr<FntFileData> fileData = ResourceHandler::loadFntFile(filepath);

		if (fileData == nullptr) {
			LOG_ERR("Failed to load fnt file");
			return;
		}

		for (const FntFilePageData& page : fileData->Pages) {
			std::shared_ptr<TextureVulkan> pageTexture = ObjInit::texture((imageDir + page.PageFilepath).c_str());
			mPageTextures.emplace_back(pageTexture);
		}

		const float fileWidth = static_cast<float>(fileData->Width);
		const float fileHeight = static_cast<float>(fileData->Height);

		mPageCount = fileData->PageCount;
		mLineHeightNorm = fileData->LineHeight / fileHeight;
		mBaseNorm = fileData->Base / fileHeight;

		bool spaceFound = false;
		for (const FntFileGlyphData& fileGlyph : fileData->Chars) {
			if (fileGlyph.Id == 32) { //ASCII & UTF decimal value for "space"
				mSpaceWidthNorm = fileGlyph.AdvanceX / fileWidth;
				spaceFound = true;
				continue;
			}

			GlyphData g = {};
			g.PageId = fileGlyph.PageId;

			//IMPORTANT:: Invert Y-axis to align coordinate space from fnt's to DOH's
			//TODO:: When allowing for other types of font files, account for this where needed
			g.Offset = {
				fileGlyph.OffsetX / fileWidth,
				-fileGlyph.OffsetY / fileHeight
			};
			g.Size = {
				fileGlyph.Width / fileWidth,
				fileGlyph.Height / fileHeight
			};
			g.TexCoordTopLeft = {
				fileGlyph.X / fileWidth,
				fileGlyph.Y / fileHeight
			};
			g.TexCoordBotRight = {
				g.TexCoordTopLeft.x + g.Size.x,
				g.TexCoordTopLeft.y + g.Size.y
			};
			g.AdvanceX = fileGlyph.AdvanceX / fileWidth;

			mGlyphMap.emplace(fileGlyph.Id, g);
		}

		if (!spaceFound) {
			mSpaceWidthNorm = 8.0f / fileWidth;
		}

		for (const FntFileKerningData& fileKerning : fileData->Kernings) {
			KerningData k = {
				fileKerning.FirstGlyphId,
				fileKerning.SecondGlyphId,
				fileKerning.Amount / fileWidth
			};
			mKernings.emplace_back(k);
		}
	}
}
