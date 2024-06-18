#include "dough/files/readers/FntFileReader.h"

#include "dough/files/ResourceHandler.h"
#include "dough/Logging.h"
#include "dough/files/TextFileUtils.h"

#include <tracy/public/tracy/Tracy.hpp>

#define DOH_FNT_MAX_STRING_CHAR_LENGTH 128
#define DOH_FNT_MAX_INT_CHAR_LENGTH 8

namespace DOH {

	FntFileReader::FntFileReader(const char* filepath, const bool openNow)
	:	AFileReader(filepath)
	{
		ZoneScoped;

		bool isValid = true;
		if (!ResourceHandler::isFileOfType(filepath, "fnt")) {
			LOG_ERR("Invalid file type for reader (.fnt): " << filepath)
			isValid = false;
		}

		if (isValid && openNow) {
			if (!open()) {
				LOG_ERR("Failed to open file: " << filepath);
			}
		}
	}

	const bool FntFileReader::open() {
		ZoneScoped;

		if (!mOpen) {
			mChars = ResourceHandler::readFile(mFilepath);
			mOpen = true;
		} else {
			LOG_WARN("Attempting open already open reader");
		}

		return mOpen;
	}

	std::shared_ptr<FntFileData> FntFileReader::read(const bool closeWhenRead) {
		ZoneScoped;

		if (!mOpen) {
			LOG_ERR("Attempting to read fnt file when not open");
			return nullptr;
		}

		std::shared_ptr<FntFileData> fntFileData = std::make_shared<FntFileData>();

		size_t currentIndex = 0;

		//Read the pseudo "header" of an .fnt file, "info" & "common" lines
		//Check for if on "info" line
		if (mChars[currentIndex] == 'i') {
			const std::string lineBuffer = ResourceHandler::getCurrentLineAsBuffer(mChars, currentIndex);

			const std::pair<std::string, size_t>& faceValue = TextFileUtils::readElementString(lineBuffer, "face", 0); //Offset to ignore "info"
			const std::pair<int, size_t>& sizeValue = TextFileUtils::readElementInt(lineBuffer, "size", faceValue.second);

			fntFileData->Face = faceValue.first;
			fntFileData->CharSize = static_cast<uint32_t>(sizeValue.first);

			//TODO:: bold? italic? charset? unicode? stretchH? smooth? aa? padding? spacing? outline?

			currentIndex += lineBuffer.size();
		} else {
			LOG_ERR("Failed to read fnt file. \"common\" line was not found.");
			return nullptr;
		}

		//Check for if on "common" line
		if (mChars[currentIndex + 1] == 'o') {
			const std::string lineBuffer = ResourceHandler::getCurrentLineAsBuffer(mChars, currentIndex);

			const std::pair<int, size_t>& lineHeightValue = TextFileUtils::readElementInt(lineBuffer, "lineHeight", 0);
			const std::pair<int, size_t>& baseValue = TextFileUtils::readElementInt(lineBuffer, "base", lineHeightValue.second);
			const std::pair<int, size_t>& widthValue = TextFileUtils::readElementInt(lineBuffer, "scaleW", baseValue.second);
			const std::pair<int, size_t>& heightValue = TextFileUtils::readElementInt(lineBuffer, "scaleH", widthValue.second);
			const std::pair<int, size_t>& pagesValue = TextFileUtils::readElementInt(lineBuffer, "pages", heightValue.second);

			fntFileData->LineHeight = static_cast<uint32_t>(lineHeightValue.first);
			fntFileData->Base = static_cast<uint32_t>(baseValue.first);
			fntFileData->Width = static_cast<uint32_t>(widthValue.first);
			fntFileData->Height = static_cast<uint32_t>(heightValue.first);
			fntFileData->PageCount = static_cast<uint32_t>(pagesValue.first);

			fntFileData->Pages.reserve(fntFileData->PageCount);

			currentIndex += lineBuffer.size();
		} else {
			LOG_ERR("Failed to read fnt file. \"info\" line was not found.");
			return nullptr;
		}

		for (uint32_t i = 0; i < fntFileData->PageCount; i++) {
			//Read page, "page" definition, "chars" count and individual "char" lines
			//Check for if on "page" line
			if (mChars[currentIndex] == 'p') {
				const std::string lineBuffer = ResourceHandler::getCurrentLineAsBuffer(mChars, currentIndex);

				const std::pair<int, size_t>& idValue = TextFileUtils::readElementInt(lineBuffer, "id", 0);
				const std::pair<std::string, size_t>& fileValue = TextFileUtils::readElementString(lineBuffer, "file", idValue.second);

				FntFilePageData pageData = { fileValue.first, static_cast<uint32_t>(idValue.first) };
				fntFileData->Pages.emplace_back(pageData);
				currentIndex += lineBuffer.size();
			} else {
				LOG_ERR("Failed to read fnt file. \"page\" line was not found.");
				return nullptr;
			}
		}

		//Check for if on "chars" line
		if (mChars[currentIndex + 4] == 's') {
			const std::string lineBuffer = ResourceHandler::getCurrentLineAsBuffer(mChars, currentIndex);
			const std::pair<int, size_t>& countValue = TextFileUtils::readElementInt(lineBuffer, "count", 0);

			if (countValue.first < 1) {
				LOG_ERR("fnt file char count < 1");
				return nullptr;
			}

			fntFileData->CharCount = static_cast<uint32_t>(countValue.first);
			fntFileData->Chars.reserve(static_cast<size_t>(countValue.first));

			currentIndex += lineBuffer.size();
		} else {
			LOG_ERR("Failed to read fnt file. \"chars\" line was not found.");
			return nullptr;
		}

		for (uint32_t i = 0; i < fntFileData->CharCount; i++) {
			const std::string lineBuffer = ResourceHandler::getCurrentLineAsBuffer(mChars, currentIndex);

			const std::pair<int, size_t>& idValue = TextFileUtils::readElementInt(lineBuffer, "id", 0);
			const std::pair<int, size_t>& xValue = TextFileUtils::readElementInt(lineBuffer, "x", idValue.second);
			const std::pair<int, size_t>& yValue = TextFileUtils::readElementInt(lineBuffer, "y", xValue.second);
			const std::pair<int, size_t>& widthValue = TextFileUtils::readElementInt(lineBuffer, "width", yValue.second);
			const std::pair<int, size_t>& heightValue = TextFileUtils::readElementInt(lineBuffer, "height", widthValue.second);
			const std::pair<int, size_t>& xOffsetValue = TextFileUtils::readElementInt(lineBuffer, "xoffset", heightValue.second);
			const std::pair<int, size_t>& yOffsetValue = TextFileUtils::readElementInt(lineBuffer, "yoffset", xOffsetValue.second);
			const std::pair<int, size_t>& xAdvanceValue = TextFileUtils::readElementInt(lineBuffer, "xadvance", yOffsetValue.second);
			const std::pair<int, size_t>& pageValue = TextFileUtils::readElementInt(lineBuffer, "page", xAdvanceValue.second);

			FntFileGlyphData charData = {
				static_cast<uint32_t>(idValue.first),
				static_cast<uint32_t>(xValue.first),
				static_cast<uint32_t>(yValue.first),
				static_cast<uint32_t>(widthValue.first),
				static_cast<uint32_t>(heightValue.first),
				xOffsetValue.first,
				yOffsetValue.first,
				static_cast<uint32_t>(xAdvanceValue.first),
				static_cast<uint32_t>(pageValue.first)
			};
			fntFileData->Chars.emplace_back(charData);
			currentIndex += lineBuffer.size();
		}

		if (currentIndex != mChars.size()) {
			//Read kernings count
			//Check for if on "kernings" line
			if (mChars[currentIndex + 7] == 's') {
				const std::string lineBuffer = ResourceHandler::getCurrentLineAsBuffer(mChars, currentIndex);

				const std::pair<int, size_t>& countValue = TextFileUtils::readElementInt(lineBuffer, "count", 0);
				fntFileData->KerningCount = static_cast<uint32_t>(countValue.first);

				currentIndex += lineBuffer.size();
			} else {
				LOG_ERR("Failed to read fnt file. \"kernings\" line was not found.");
				return nullptr;
			}

			for (uint32_t i = 0; i < fntFileData->KerningCount; i++) {
				const std::string lineBuffer = ResourceHandler::getCurrentLineAsBuffer(mChars, currentIndex);

				const std::pair<int, size_t>& firstValue = TextFileUtils::readElementInt(lineBuffer, "first", 0);
				const std::pair<int, size_t>& secondValue = TextFileUtils::readElementInt(lineBuffer, "second", firstValue.second);
				const std::pair<int, size_t>& amountValue = TextFileUtils::readElementInt(lineBuffer, "amount", secondValue.second);

				FntFileKerningData kerningData = {
					static_cast<uint32_t>(firstValue.first),
					static_cast<uint32_t>(secondValue.first),
					amountValue.first
				};
				fntFileData->Kernings.emplace_back(kerningData);

				currentIndex += lineBuffer.size();
			}
		}

		if (closeWhenRead) {
			close();
		}

		return fntFileData;
	}
}
