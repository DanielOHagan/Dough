#include "dough/files/readers/FntFileReader.h"

#include "dough/files/ResourceHandler.h"
#include "dough/Logging.h"

#define DOH_FNT_MAX_STRING_CHAR_LENGTH 128
#define DOH_FNT_MAX_INT_CHAR_LENGTH 8

namespace DOH {

	FntFileReader::FntFileReader(const char* filepath, const bool openNow)
	:	AFileReader(filepath)
	{
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
		if (!mOpen) {
			mChars = ResourceHandler::readFile(mFilepath);
			mOpen = true;
		} else {
			LOG_WARN("Attempting open already open reader");
		}

		return mOpen;
	}

	std::shared_ptr<FntFileData> FntFileReader::read(const bool closeWhenRead) {
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

			const std::pair<std::string, size_t>& faceValue = FntFileReader::readFntFileElementString(lineBuffer, "face", 0); //Offset to ignore "info"
			const std::pair<int, size_t>& sizeValue = FntFileReader::readFntFileElementInt(lineBuffer, "size", faceValue.second);

			fntFileData->Face = faceValue.first;
			fntFileData->CharSize = static_cast<uint32_t>(sizeValue.first);

			//TODO:: bold? italic? charset? unicode? stretchH? smooth? aa? padding? spacing? outline?

			//Demonstration & testing examples
			//const auto& paddingValuesTest = readFntFileElementMultipleInt(
			//	lineBuffer,
			//	"padding",
			//	4,
			//	sizeValue.second
			//);
			//const auto& spacingValuesTest = readFntFileElementMultipleInt(
			//	lineBuffer,
			//	"spacing",
			//	2,
			//	paddingValuesTest.second
			//);
			//const auto& outlineTest = readFntFileElementInt(lineBuffer, "outline", spacingValuesTest.second);

			currentIndex += lineBuffer.size();
		} else {
			LOG_ERR("Failed to read fnt file. \"common\" line was not found.");
			return nullptr;
		}

		//Check for if on "common" line
		if (mChars[currentIndex + 1] == 'o') {
			const std::string lineBuffer = ResourceHandler::getCurrentLineAsBuffer(mChars, currentIndex);

			const std::pair<int, size_t>& lineHeightValue = FntFileReader::readFntFileElementInt(lineBuffer, "lineHeight", 0);
			const std::pair<int, size_t>& baseValue = FntFileReader::readFntFileElementInt(lineBuffer, "base", lineHeightValue.second);
			const std::pair<int, size_t>& widthValue = FntFileReader::readFntFileElementInt(lineBuffer, "scaleW", baseValue.second);
			const std::pair<int, size_t>& heightValue = FntFileReader::readFntFileElementInt(lineBuffer, "scaleH", widthValue.second);
			const std::pair<int, size_t>& pagesValue = FntFileReader::readFntFileElementInt(lineBuffer, "pages", heightValue.second);

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

				const std::pair<int, size_t>& idValue = readFntFileElementInt(lineBuffer, "id", 0);
				const std::pair<std::string, size_t>& fileValue = readFntFileElementString(lineBuffer, "file", idValue.second);

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
			const std::pair<int, size_t>& countValue = readFntFileElementInt(lineBuffer, "count", 0);

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

			const std::pair<int, size_t>& idValue = FntFileReader::readFntFileElementInt(lineBuffer, "id", 0);
			const std::pair<int, size_t>& xValue = FntFileReader::readFntFileElementInt(lineBuffer, "x", idValue.second);
			const std::pair<int, size_t>& yValue = FntFileReader::readFntFileElementInt(lineBuffer, "y", xValue.second);
			const std::pair<int, size_t>& widthValue = FntFileReader::readFntFileElementInt(lineBuffer, "width", yValue.second);
			const std::pair<int, size_t>& heightValue = FntFileReader::readFntFileElementInt(lineBuffer, "height", widthValue.second);
			const std::pair<int, size_t>& xOffsetValue = FntFileReader::readFntFileElementInt(lineBuffer, "xoffset", heightValue.second);
			const std::pair<int, size_t>& yOffsetValue = FntFileReader::readFntFileElementInt(lineBuffer, "yoffset", xOffsetValue.second);
			const std::pair<int, size_t>& xAdvanceValue = FntFileReader::readFntFileElementInt(lineBuffer, "xadvance", yOffsetValue.second);
			const std::pair<int, size_t>& pageValue = FntFileReader::readFntFileElementInt(lineBuffer, "page", xAdvanceValue.second);

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

				const std::pair<int, size_t>& countValue = FntFileReader::readFntFileElementInt(lineBuffer, "count", 0);
				fntFileData->KerningCount = static_cast<uint32_t>(countValue.first);

				currentIndex += lineBuffer.size();
			} else {
				LOG_ERR("Failed to read fnt file. \"kernings\" line was not found.");
				return nullptr;
			}

			for (uint32_t i = 0; i < fntFileData->KerningCount; i++) {
				const std::string lineBuffer = ResourceHandler::getCurrentLineAsBuffer(mChars, currentIndex);

				const std::pair<int, size_t>& firstValue = FntFileReader::readFntFileElementInt(lineBuffer, "first", 0);
				const std::pair<int, size_t>& secondValue = FntFileReader::readFntFileElementInt(lineBuffer, "second", firstValue.second);
				const std::pair<int, size_t>& amountValue = FntFileReader::readFntFileElementInt(lineBuffer, "amount", secondValue.second);

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

	std::pair<int, size_t> FntFileReader::readFntFileElementInt(
		const std::string& lineBuffer,
		const char* elementName,
		const size_t startIndex
	) {
		if (elementName == "") {
			LOG_ERR("Invalid search argument for readFntFileElementInt. elementName: " << elementName);
			return { INT16_MIN, startIndex };
		}

		//In .fnt files, integer values start after the element's = sign and end at the next empty space

		//Values will be truncated at DOH_FNT_MAX_INT_CHAR_LENGTH - 1 digits, leaving room for zero-termination char
		char valueChars[DOH_FNT_MAX_INT_CHAR_LENGTH] = {};

		size_t targetElementStartIndex = lineBuffer.find(elementName, startIndex);
		if (targetElementStartIndex == std::string::npos) {
			LOG_ERR("readFntFileElementInt failed to find element: " << elementName);
			return { INT16_MIN, startIndex };
		}
		targetElementStartIndex += strlen(elementName) + 1; //Include offset of element name and =

		//TODO:: is there a faster way of getting the first non numerical value? instead of an equals to check for each char,
		//	shouldn't a < (char) 0 OR > (char) 9 with an OR for - symbal be much faster?
		const size_t targetElementEndIndex = lineBuffer.find_first_not_of("-0123456789", targetElementStartIndex);
		size_t valueCharCount = targetElementEndIndex - targetElementStartIndex;

		memcpy(
			&valueChars,
			lineBuffer.data() + targetElementStartIndex,
			sizeof(char) * valueCharCount
		);

		if (valueCharCount >= DOH_FNT_MAX_INT_CHAR_LENGTH) {
			LOG_WARN("int valueCharCount of: " << valueCharCount << " must be smaller than: " << DOH_FNT_MAX_INT_CHAR_LENGTH);
			//Truncate to ensure space for zero-termination character
			valueCharCount = DOH_FNT_MAX_INT_CHAR_LENGTH - 1;
		}
		valueChars[valueCharCount] = '\0';

		return { std::stoi(valueChars), targetElementEndIndex };
	}

	std::pair<std::array<int, 4>, size_t> FntFileReader::readFntFileElementMultipleInt(
		const std::string& lineBuffer,
		const char* elementName,
		const uint32_t intCount,
		const size_t startIndex
	) {
		if (intCount > 4 || intCount == 0 || elementName == "") {
			LOG_ERR(
				"Invalid search arguments for readFntFileElementMultipleInt. elementName: " <<
				elementName << " intCount: " << intCount
			);
			return { { INT16_MIN, INT16_MIN, INT16_MIN, INT16_MIN }, startIndex };
		}

		//In .fnt files, integer values start after the element's = sign, and end at the next empty space.
		//	Elements containing multiple integer values use a , to separate values
		size_t targetElementStartIndex = lineBuffer.find(elementName, startIndex);
		if (targetElementStartIndex == std::string::npos) {
			LOG_ERR("readFntFileElementMultipleInt failed to find element: " << elementName);
			return { { INT16_MIN, INT16_MIN, INT16_MIN, INT16_MIN }, startIndex };
		}
		targetElementStartIndex += strlen(elementName) + 1;

		size_t targetElementEndIndex = 0;
		int values[4] = {};
		bool finalValue = false;
		for (uint32_t i = 0; i < intCount; i++) {
			if (finalValue) {
				break;
			}

			char valueChars[DOH_FNT_MAX_INT_CHAR_LENGTH];

			targetElementEndIndex = lineBuffer.find_first_of(" ,", targetElementStartIndex) + 1;
			size_t valueCharCount = targetElementEndIndex - targetElementStartIndex;

			memcpy(
				&valueChars,
				lineBuffer.data() + targetElementStartIndex,
				sizeof(char) * valueCharCount
			);
			finalValue = valueChars[valueCharCount - 1] == ' ';

			if (valueCharCount >= DOH_FNT_MAX_INT_CHAR_LENGTH) {
				LOG_WARN("int valueCharCount of: " << valueCharCount << " must be smaller than: " << DOH_FNT_MAX_INT_CHAR_LENGTH);
				//Truncate to ensure space for zero-termination character
				valueCharCount = DOH_FNT_MAX_INT_CHAR_LENGTH - 1;
			}
			valueChars[valueCharCount] = '\0';

			values[i] = std::stoi(valueChars);
			targetElementStartIndex = targetElementEndIndex;
		}

		return { {values[0], values[1], values[2], values[3]}, targetElementEndIndex };
	}

	std::pair<std::string, size_t> FntFileReader::readFntFileElementString(
		const std::string& lineBuffer,
		const char* elementName,
		const size_t startIndex
	) {
		if (elementName == "") {
			LOG_ERR("Invalid search arguments for readFntFileElementString elementName: " << elementName);
			return { "", startIndex };
		}

		//In .fnt files strings are decorated with " marks at the start and end

		//Values will be truncated at DOH_FNT_MAX_STRING_CHAR_LENGTH - 1, leaving room for zero-termination char
		char valueChars[DOH_FNT_MAX_STRING_CHAR_LENGTH];

		size_t targetElementStartIndex = lineBuffer.find(elementName, startIndex);
		if (targetElementStartIndex == std::string::npos) {
			LOG_ERR("readFntFileElementString failed to find element: " << elementName);
			return { "", startIndex };
		}

		targetElementStartIndex += strlen(elementName) + 2; //Include offset of element name , = and first "
		const size_t targetElementEndIndex = lineBuffer.find("\"", targetElementStartIndex);
		size_t valueCharCount = targetElementEndIndex - targetElementStartIndex;

		memcpy(
			&valueChars,
			lineBuffer.data() + targetElementStartIndex,
			sizeof(char) * valueCharCount
		);

		if (valueCharCount >= DOH_FNT_MAX_STRING_CHAR_LENGTH) {
			LOG_WARN("int valueCharCount of: " << valueCharCount << " must be smaller than: " << DOH_FNT_MAX_STRING_CHAR_LENGTH);
			//Truncate to ensure space for zero-termination character
			valueCharCount = DOH_FNT_MAX_STRING_CHAR_LENGTH - 1;
		}
		valueChars[valueCharCount] = '\0';

		return { valueChars, targetElementEndIndex };
	}
}
