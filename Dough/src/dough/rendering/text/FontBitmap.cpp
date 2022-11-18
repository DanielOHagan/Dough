#include "dough/rendering/text/FontBitmap.h"

#include "dough/ResourceHandler.h"
#include "dough/Logging.h"
#include "dough/scene/geometry/Quad.h"
#include "dough/rendering/ObjInit.h"

namespace DOH {

	FontBitmap::FontBitmap(const char* filepath, const char* imageDir)
	:	mSpaceWidthPx(0)
	{
		std::shared_ptr<FntFileData> fileData = ResourceHandler::loadFntFile(filepath);

		if (fileData == nullptr) {
			LOG_ERR("Failed to load fnt file");
			return;
		}

		mFntFileData = fileData;
		mFntFileData->ImageDir = imageDir;

		//Set a Space length for easy reference
		const auto& spaceChar = mFntFileData->Chars.find(32); //ASCII & UTF decimal value for "space"
		if (spaceChar != mFntFileData->Chars.end()) {
			mSpaceWidthPx = spaceChar->second.AdvanceX;
		}

		for (const FntFilePageData& page : mFntFileData->Pages) {
			std::shared_ptr<TextureVulkan> pageTexture = ObjInit::texture((fileData->ImageDir + page.PageFilepath).c_str());
			mPageTextures.emplace_back(pageTexture);
		}
	}
}
