#pragma once

#include "dough/model/AFileData.h"
#include "dough/Core.h"

namespace DOH {

	struct FntFileGlyphData {
		uint32_t Id;
		uint32_t X;
		uint32_t Y;
		uint32_t Width;
		uint32_t Height;
		int OffsetX;
		int OffsetY;
		uint32_t AdvanceX;
		uint32_t PageId;
	};

	struct FntFileKerningData {
		uint32_t FirstGlyphId;
		uint32_t SecondGlyphId;
		int Amount;
	};

	struct FntFilePageData {
		std::string PageFilepath;
		uint32_t PageId;
	};

	struct FntFileData : public AFileData {

	public:
		FntFileData()
		:	AFileData(),
			CharSize(0),
			CharCount(0),
			KerningCount(0),
			PageCount(0),
			LineHeight(0),
			Base(0),
			Width(0),
			Height(0)
		{};

		std::vector<FntFilePageData> Pages;
		std::vector<FntFileGlyphData> Chars;
		std::vector<FntFileKerningData> Kernings;
		std::string Face;
		std::string ImageDir;
		uint32_t CharSize;
		uint32_t CharCount;
		uint32_t KerningCount;
		uint32_t PageCount;
		uint32_t LineHeight;
		uint32_t Base;
		uint32_t Width;
		uint32_t Height;
	};
}
