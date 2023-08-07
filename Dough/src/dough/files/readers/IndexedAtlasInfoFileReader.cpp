#include "dough/files/readers/IndexedAtlasInfoFileReader.h"

#include "dough/files/ResourceHandler.h"
#include "dough/Logging.h"
#include "dough/rendering/textures/TextureAtlas.h"

namespace DOH {

	IndexedAtlasInfoFileReader::IndexedAtlasInfoFileReader(const char* atlastInfoFilePath, const bool openNow)
	:	AFileReader(atlastInfoFilePath)
	{
		bool isValid = true;
		if (!ResourceHandler::isFileOfType(atlastInfoFilePath, "txt")) {
			LOG_ERR("Invalid file type for reader (.txt): " << atlastInfoFilePath)
			isValid = false;
		}

		if (isValid && openNow) {
			if (!open()) {
				LOG_ERR("Failed to open file: " << atlastInfoFilePath);
			}
		}
	}

	const bool IndexedAtlasInfoFileReader::open() {
		if (!mOpen) {
			mChars = ResourceHandler::readFile(mFilepath);
			mOpen = true;
		} else {
			LOG_WARN("Attempting open already open reader");
		}

		return mOpen;
	}

	std::shared_ptr<IndexedAtlasInfoFileData> IndexedAtlasInfoFileReader::read(const bool closeWhenRead) {
		//TODO:: Currently using a custom format to describe indexed texture atlasses, will likely have a system to handle texture atlasses from other file types.

		if (!mOpen) {
			LOG_ERR("Attempting to read IndexedAtlasInfo file when not open");
			return nullptr;
		}

		std::shared_ptr<IndexedAtlasInfoFileData> atlasInfoFileData = std::make_shared<IndexedAtlasInfoFileData>();

		size_t currentIndex = 0;

		float width = 0.0f;
		float height = 0.0f;

		//Check for if on line that contains "name" & "textureFilePath"
		if (mChars[currentIndex] == 'n') {
			const std::string lineBuffer = ResourceHandler::getCurrentLineAsBuffer(mChars, currentIndex);

			const std::pair<std::string, size_t>& atlasNameValue = IndexedAtlasInfoFileReader::readElementString(lineBuffer, "name", 0);
			const std::pair<std::string, size_t>& textureFilePathValue = IndexedAtlasInfoFileReader::readElementString(lineBuffer, "textureFilePath", atlasNameValue.second);
			const std::pair<int, size_t>& widthValue = IndexedAtlasInfoFileReader::readElementInt(lineBuffer, "w", textureFilePathValue.second);
			const std::pair<int, size_t>& heightValue = IndexedAtlasInfoFileReader::readElementInt(lineBuffer, "h", widthValue.second);

			atlasInfoFileData->Name = atlasNameValue.first;
			atlasInfoFileData->TextureFileName = textureFilePathValue.first;
			width = static_cast<float>(widthValue.first);
			height = static_cast<float>(heightValue.first);

			currentIndex += lineBuffer.size();
		} else {
			LOG_ERR("Failed to read IndexedAtlasInfo file. Line containing \"name\" & \"textureFilePath\"");
			return nullptr;
		}

		//Check for if on "innerTextureCount" & "innerTextures" line
		if (mChars[currentIndex] == 'i') {
			const std::string lineBuffer = ResourceHandler::getCurrentLineAsBuffer(mChars, currentIndex);

			const std::pair<int, size_t>& innerTextureCountValue = IndexedAtlasInfoFileReader::readElementInt(lineBuffer, "innerTextureCount", 0);

			atlasInfoFileData->InnerTextureCount = static_cast<uint32_t>(innerTextureCountValue.first);

			currentIndex += lineBuffer.size();
		} else {
			LOG_ERR("Failed to read IndexedAtlasInfo file. Line containing \"innerTextures\" not found.");
			return nullptr;
		}

		atlasInfoFileData->InnerTextures.reserve(atlasInfoFileData->InnerTextureCount);
		//Get innerTexture info
		for (uint32_t i = 0; i < atlasInfoFileData->InnerTextureCount; i++) {
			const std::string lineBuffer = ResourceHandler::getCurrentLineAsBuffer(mChars, currentIndex);

			const std::pair<std::string, size_t>& iNameValue = IndexedAtlasInfoFileReader::readElementString(lineBuffer, "iName", 0);
			const std::pair<std::array<int, IndexedAtlasInfoFileReader::MAX_MULTIPLE_INT_COUNT>, size_t>& texelCoordsTlValue =
				readElementIntVector(lineBuffer, "tl", iNameValue.second, 2);
			const std::pair<std::array<int, IndexedAtlasInfoFileReader::MAX_MULTIPLE_INT_COUNT>, size_t>& texelCoordsTrValue =
				readElementIntVector(lineBuffer, "tr", texelCoordsTlValue.second, 2);
			const std::pair<std::array<int, IndexedAtlasInfoFileReader::MAX_MULTIPLE_INT_COUNT>, size_t>& texelCoordsBrValue =
				readElementIntVector(lineBuffer, "br", texelCoordsTrValue.second, 2);
			const std::pair<std::array<int, IndexedAtlasInfoFileReader::MAX_MULTIPLE_INT_COUNT>, size_t>& texelCoordsBlValue = 
				readElementIntVector(lineBuffer, "bl", texelCoordsBrValue.second, 2);

			std::array<uint32_t, 8> texelCoords = {
				//Top Left
				static_cast<uint32_t>(texelCoordsTlValue.first[0]),
				static_cast<uint32_t>(texelCoordsTlValue.first[1]),
				//Top Right
				static_cast<uint32_t>(texelCoordsTrValue.first[0]),
				static_cast<uint32_t>(texelCoordsTrValue.first[1]),
				//Bottom Right
				static_cast<uint32_t>(texelCoordsBrValue.first[0]),
				static_cast<uint32_t>(texelCoordsBrValue.first[1]),
				//Bottom Left
				static_cast<uint32_t>(texelCoordsBlValue.first[0]),
				static_cast<uint32_t>(texelCoordsBlValue.first[1])
			};
			std::array<float, 8> textureCoords = {
				//Top Left
				texelCoords[0] > 0 ? static_cast<float>(texelCoords[0]) / width : 0.0f,
				texelCoords[1] > 0 ? static_cast<float>(texelCoords[1]) / height : 0.0f,
				//Top Right
				texelCoords[2] > 0 ? static_cast<float>(texelCoords[2]) / width : 0.0f,
				texelCoords[3] > 0 ? static_cast<float>(texelCoords[3]) / height : 0.0f,
				//Bottom Right
				texelCoords[4] > 0 ? static_cast<float>(texelCoords[4]) / width : 0.0f,
				texelCoords[5] > 0 ? static_cast<float>(texelCoords[5]) / height : 0.0f,
				//Bottom Left
				texelCoords[6] > 0 ? static_cast<float>(texelCoords[6]) / width : 0.0f,
				texelCoords[7] > 0 ? static_cast<float>(texelCoords[7]) / height : 0.0f
			};

			{
				//Convert Y-axis values to Vulkan texture coords space
				float topY1 = textureCoords[1];
				float topY2 = textureCoords[3];
				float botY1 = textureCoords[5];
				float botY2 = textureCoords[7];

				textureCoords[1] = botY1;
				textureCoords[3] = botY2;
				textureCoords[5] = topY1;
				textureCoords[7] = topY2;
			}

			//InnerTexture innerTexture = { iNameValue.first, texCoords };
			InnerTexture innerTexture = { texelCoords, textureCoords };

			atlasInfoFileData->InnerTextures.emplace(iNameValue.first, innerTexture);
			currentIndex += lineBuffer.size();
		}

		return atlasInfoFileData;
	}

	const std::pair<std::string, size_t> IndexedAtlasInfoFileReader::readElementString(
		const std::string& lineBuffer,
		const char* elementName,
		size_t startIndex
	) {
		const size_t elementNameLength = strlen(elementName);

		if (elementNameLength == 0) {
			LOG_ERR("Invalid search argument for readElementString. elementName: " << elementName);
			return { "", startIndex };
		}

		//Values will be truncated at MAX_STRING_CHAR_LENGTH - 1, leaving room for zero-termination char
		char valueChars[IndexedAtlasInfoFileReader::MAX_STRING_CHAR_LENGTH] = {};

		size_t targetElementStartIndex = lineBuffer.find(elementName, startIndex);
		if (targetElementStartIndex == std::string::npos) {
			LOG_ERR("Failed to find element: " << elementName);
			return { "", startIndex };
		}

		targetElementStartIndex += elementNameLength + 2; //Include offset of element name, '=' and first '"'
		const size_t targetElementEndIndex = lineBuffer.find("\"", targetElementStartIndex);
		size_t valueCharCount = targetElementEndIndex - targetElementStartIndex;

		if (valueCharCount >= IndexedAtlasInfoFileReader::MAX_STRING_CHAR_LENGTH) {
			LOG_WARN(
				"Length of string element: " << valueCharCount <<
				" must be smaller than: " << IndexedAtlasInfoFileReader::MAX_STRING_CHAR_LENGTH << " truncating to accommodate."
			);

			valueCharCount = static_cast<size_t>(IndexedAtlasInfoFileReader::MAX_STRING_CHAR_LENGTH - 1);
		}

		memcpy(
			&valueChars,
			lineBuffer.data() + targetElementStartIndex,
			sizeof(char) * valueCharCount
		);

		valueChars[valueCharCount] = '\0';

		return { valueChars, targetElementEndIndex }; //TODO:: targetElementEndIndex + 1? To account for the 2nd speach marks
	}

	const std::pair<int, size_t> IndexedAtlasInfoFileReader::readElementInt(
		const std::string& lineBuffer,
		const char* elementName,
		size_t startIndex
	) {
		const size_t elementNameLength  = strlen(elementName);

		if (elementNameLength == 0) {
			LOG_ERR("Invalid search argument for readElementInt. elementName: " << elementName);
			return { INT16_MIN, startIndex };
		}

		//Values will be truncated at MAX_INT_CHAR_LENGTH - 1 digits, leaving room for zero-termination char
		char valueChars[IndexedAtlasInfoFileReader::MAX_INT_CHAR_LENGTH] = {};

		size_t targetElementStartIndex = lineBuffer.find(elementName, startIndex);
		if (targetElementStartIndex == std::string::npos) {
			LOG_ERR("readElementInt failed to find element: " << elementName);
			return { INT16_MIN, startIndex };
		}
		targetElementStartIndex += elementNameLength + 1; //Include offset of element name and '='

		//TODO:: is there a faster way of getting the first non numerical value? instead of an equals to check for each char,
		//	shouldn't a < (char) 0 OR > (char) 9 with an OR for - symbal be much faster?
		const size_t targetElementEndIndex = lineBuffer.find_first_not_of("-0123456789", targetElementStartIndex);
		size_t valueCharCount = targetElementEndIndex - targetElementStartIndex;

		if (valueCharCount >= IndexedAtlasInfoFileReader::MAX_INT_CHAR_LENGTH) {
			LOG_WARN(
				"int valueCharCount of: " << valueCharCount <<
				" must be smaller than: " << IndexedAtlasInfoFileReader::MAX_INT_CHAR_LENGTH
			);
			//Truncate to ensure space for zero-termination character
			valueCharCount = static_cast<size_t>(IndexedAtlasInfoFileReader::MAX_INT_CHAR_LENGTH - 1);
		}
		valueChars[valueCharCount] = '\0';

		memcpy(
			&valueChars,
			lineBuffer.data() + targetElementStartIndex,
			sizeof(char) * valueCharCount
		);

		return { std::stoi(valueChars), targetElementStartIndex };
	}

	const std::pair<std::array<int, IndexedAtlasInfoFileReader::MAX_MULTIPLE_INT_COUNT>, size_t> IndexedAtlasInfoFileReader::readElementIntVector(
		const std::string& lineBuffer,
		const char* elementName,
		size_t startIndex,
		uint32_t valueCount
	) {
		const size_t elementNameLength = strlen(elementName);

		if (valueCount > IndexedAtlasInfoFileReader::MAX_MULTIPLE_INT_COUNT || valueCount == 0 || elementNameLength == 0) {
			LOG_ERR(
				"Invalid search arguments for readElementIntVector. elementName " << elementName <<
				" valueCount: " << valueCount
			);
			return { { INT16_MIN, INT16_MIN, INT16_MIN, INT16_MIN }, startIndex };
		}

		size_t targetElementStartIndex = lineBuffer.find(elementName, startIndex);
		if (targetElementStartIndex == std::string::npos) {
			LOG_ERR("readElementIntVector failed to find element: " << elementName);
			return { { INT16_MIN, INT16_MIN, INT16_MIN, INT16_MIN }, startIndex };
		}
		targetElementStartIndex += elementNameLength + 1; //Inlcude + 1 for the '='

		size_t targetElementEndIndex = 0;
		int values[IndexedAtlasInfoFileReader::MAX_MULTIPLE_INT_COUNT] = {};
		bool finalValue = false;
		for (uint32_t i  = 0; i < valueCount; i++) {
			if (finalValue) {
				LOG_WARN("Found end of vector before given valueCount reached. Found: " << i << " of desired: " << valueCount);
				break;
			}

			char valueChars[IndexedAtlasInfoFileReader::MAX_INT_CHAR_LENGTH] = {};
			targetElementEndIndex = lineBuffer.find_first_not_of("0123456789", targetElementStartIndex) + 1;
			size_t valueCharCount = targetElementEndIndex - targetElementStartIndex;

			if (valueCharCount >= IndexedAtlasInfoFileReader::MAX_FLOAT_CHAR_LENGTH) {
				LOG_WARN(
					"readElementIntVector. valueCharCount: " << valueCharCount <<
					" must be smaller than max: " << IndexedAtlasInfoFileReader::MAX_INT_CHAR_LENGTH
				);
				valueCharCount = static_cast<size_t>(IndexedAtlasInfoFileReader::MAX_INT_CHAR_LENGTH - 1);
			} else if (valueCharCount == 0) {
				LOG_WARN("readElementIntVector. Found empty value when searching for value number: " << i + 1);
				targetElementStartIndex = targetElementEndIndex;
				continue;
			}

			memcpy(
				&valueChars,
				lineBuffer.data() + targetElementStartIndex,
				sizeof(char) * valueCharCount
			);
			finalValue = valueChars[valueCharCount - 1] == ' ';

			valueChars[valueCharCount] = '\0';

			values[i] = std::stoi(valueChars);
			targetElementStartIndex = targetElementEndIndex;
		}

		return { { values[0], values[1], values[2], values[3] }, targetElementEndIndex };
	}

	const std::pair<float, size_t> IndexedAtlasInfoFileReader::readElementFloat(
		const std::string& lineBuffer,
		const char* elementName,
		size_t startIndex
	) {
		const size_t elementNameLength = strlen(elementName);

		if (elementNameLength > 0) {
			LOG_ERR("Invalid search arguments for readElementFloat. elementName is empty.");
			return { FLT_MIN , startIndex };
		}

		size_t targetElementStartIndex = lineBuffer.find(elementName, startIndex);
		if (targetElementStartIndex == std::string::npos) {
			LOG_ERR("readElementFloat failed to find element: " << elementName);
			return { FLT_MIN, startIndex };
		}
		targetElementStartIndex += elementNameLength + 1; //Include + 1 for the '='

		const size_t targetElementEndIndex = lineBuffer.find_first_not_of("-.0123456789", targetElementStartIndex);
		size_t valueCharCount = targetElementEndIndex - targetElementStartIndex;

		if (valueCharCount >= IndexedAtlasInfoFileReader::MAX_INT_CHAR_LENGTH) {
			LOG_WARN(
				"int valueCharCount of: " << valueCharCount <<
				" must be smaller than: " << IndexedAtlasInfoFileReader::MAX_INT_CHAR_LENGTH
			);
			//Truncate to ensure space for zero-termination character
			valueCharCount = static_cast<size_t>(IndexedAtlasInfoFileReader::MAX_INT_CHAR_LENGTH - 1);
		}

		char valueChars[MAX_FLOAT_CHAR_LENGTH] = {};

		memcpy(
			&valueChars,
			lineBuffer.data() + targetElementStartIndex,
			sizeof(char) * valueCharCount
		);
		valueChars[valueCharCount] = '\0';

		return { std::stof(valueChars), targetElementStartIndex };
	}

	const std::pair<std::array<float, IndexedAtlasInfoFileReader::MAX_MULTIPLE_FLOAT_COUNT>, size_t> IndexedAtlasInfoFileReader::readElementFloatVector(
		const std::string& lineBuffer,
		const char* elementName,
		size_t startIndex,
		uint32_t valueCount
	) {
		const size_t elementNameLength = strlen(elementName);

		if (valueCount > IndexedAtlasInfoFileReader::MAX_MULTIPLE_FLOAT_COUNT || valueCount == 0 || elementNameLength == 0) {
			LOG_ERR(
				"Invalid search arguments for readElementFloatVector. elementName " << elementName <<
				" valueCount: " << valueCount
			);
			return { { FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN }, startIndex };
		}

		size_t targetElementStartIndex = lineBuffer.find(elementName, startIndex);
		if (targetElementStartIndex == std::string::npos) {
			LOG_ERR("readElementFloatVector failed to find element: " << elementName);
			return { { FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN }, startIndex };
		}
		targetElementStartIndex += elementNameLength + 1; //Include + 1 for the '='

		size_t targetElementEndIndex = 0;
		float values[IndexedAtlasInfoFileReader::MAX_FLOAT_CHAR_LENGTH] = {};
		bool finalValue = false;
		for (uint32_t i = 0; i < valueCount; i++) {
			if (finalValue) {
				LOG_WARN("Found end of vector before given valueCount reached. Found: " << i << " of desired: " << valueCount);
				break;
			}

			char valueChars[IndexedAtlasInfoFileReader::MAX_FLOAT_CHAR_LENGTH] = {};
			targetElementEndIndex = lineBuffer.find_first_not_of("-.0123456789", targetElementStartIndex) + 1;
			size_t valueCharCount = targetElementEndIndex - targetElementStartIndex;

			if (valueCharCount >= IndexedAtlasInfoFileReader::MAX_FLOAT_CHAR_LENGTH) {
				LOG_WARN(
					"readElementFloatVector. valueCharCount: " << valueCharCount <<
					" must be smaller than max: " << IndexedAtlasInfoFileReader::MAX_FLOAT_CHAR_LENGTH
				);
				valueCharCount = static_cast<size_t>(IndexedAtlasInfoFileReader::MAX_FLOAT_CHAR_LENGTH - 1);
			} else if (valueCharCount == 0) {
				LOG_WARN("readElementFloatVector. Found empty value when searching for value number: " << i + 1);
				targetElementStartIndex = targetElementEndIndex;
				continue;
				//break;
			}

			memcpy(
				&valueChars,
				lineBuffer.data() + targetElementStartIndex,
				sizeof(char) * valueCharCount
			);
			finalValue = valueChars[valueCharCount - 1] == ' ';

			valueChars[valueCharCount] = '\0';

			values[i] = std::stof(valueChars);
			targetElementStartIndex = targetElementEndIndex;
		}

		return { { values[0], values[1], values[2], values[3] }, targetElementEndIndex };
	}
}
