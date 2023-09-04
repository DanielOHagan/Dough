#include "dough/files/readers/IndexedAtlasInfoFileReader.h"

#include "dough/files/ResourceHandler.h"
#include "dough/rendering/textures/TextureAtlas.h"
#include "dough/files/TextFileUtils.h"
#include "dough/rendering/animations/TextureAtlasAnimation.h"

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

			const std::pair<std::string, size_t>& atlasNameValue = TextFileUtils::readElementString(lineBuffer, "name", 0);
			const std::pair<std::string, size_t>& textureFilePathValue = TextFileUtils::readElementString(lineBuffer, "textureFilePath", atlasNameValue.second);
			const std::pair<int, size_t>& widthValue = TextFileUtils::readElementInt(lineBuffer, "w", textureFilePathValue.second);
			const std::pair<int, size_t>& heightValue = TextFileUtils::readElementInt(lineBuffer, "h", widthValue.second);

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

			const std::pair<int, size_t>& innerTextureCountValue = TextFileUtils::readElementInt(lineBuffer, "innerTextureCount", 0);

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

			const std::pair<std::string, size_t>& iNameValue = TextFileUtils::readElementString(lineBuffer, "iName", 0);
			const std::pair<std::array<int, TextFileUtils::MAX_MULTIPLE_INT_COUNT>, size_t>& texelCoordsTlValue =
				TextFileUtils::readElementIntVector(lineBuffer, "tl", iNameValue.second, 2);
			const std::pair<std::array<int, TextFileUtils::MAX_MULTIPLE_INT_COUNT>, size_t>& texelCoordsTrValue =
				TextFileUtils::readElementIntVector(lineBuffer, "tr", texelCoordsTlValue.second, 2);
			const std::pair<std::array<int, TextFileUtils::MAX_MULTIPLE_INT_COUNT>, size_t>& texelCoordsBrValue =
				TextFileUtils::readElementIntVector(lineBuffer, "br", texelCoordsTrValue.second, 2);
			const std::pair<std::array<int, TextFileUtils::MAX_MULTIPLE_INT_COUNT>, size_t>& texelCoordsBlValue =
				TextFileUtils::readElementIntVector(lineBuffer, "bl", texelCoordsBrValue.second, 2);

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
				//Flip Y-axis values to Vulkan texture coords space
				float topY1 = textureCoords[1];
				float topY2 = textureCoords[3];
				float botY1 = textureCoords[5];
				float botY2 = textureCoords[7];

				textureCoords[1] = botY1;
				textureCoords[3] = botY2;
				textureCoords[5] = topY1;
				textureCoords[7] = topY2;
			}

			InnerTexture innerTexture = { texelCoords, textureCoords };

			atlasInfoFileData->InnerTextures.emplace(iNameValue.first, innerTexture);
			currentIndex += lineBuffer.size();
		}

		{
			const std::string lineBuffer = ResourceHandler::getCurrentLineAsBuffer(mChars, currentIndex);
			currentIndex += lineBuffer.size();

			//TODO::
			//currentIndex = TextFileUtils::getNextLineIndex(mChars, currentIndex);
		}

		//Get animation info
		if (mChars[currentIndex] == 'a') {
			const std::string lineBuffer = ResourceHandler::getCurrentLineAsBuffer(mChars, currentIndex);

			const std::pair<int, size_t>& animationCountValue = TextFileUtils::readElementInt(lineBuffer, "animationCount", 0);

			atlasInfoFileData->AnimationCount = static_cast<uint32_t>(animationCountValue.first);

			currentIndex += lineBuffer.size();
		}

		if (atlasInfoFileData->AnimationCount > 0) {
			//Get individual animation info
			atlasInfoFileData->Animations.reserve(atlasInfoFileData->AnimationCount);

			for (uint32_t i = 0; i < atlasInfoFileData->AnimationCount; i++) {
				const std::string lineBuffer = ResourceHandler::getCurrentLineAsBuffer(mChars, currentIndex);

				const std::pair<std::string, size_t>& animNameValue = TextFileUtils::readElementString(lineBuffer, "animName", 0);
				//const std::pair<std::string, size_t>& textureSequenceValue = TextFileUtils::readElementString(lineBuffer, "textureSequence", animNameValue.second);
				const std::pair<std::vector<std::string>, size_t>& textureSequenceValue =
					TextFileUtils::readElementStringList(lineBuffer, "textureSequence", animNameValue.second, TextFileUtils::LIST_READ_COUNT_ALL);
				const std::pair<int, size_t>& durationValue = TextFileUtils::readElementInt(lineBuffer, "duration", textureSequenceValue.second);
				const std::pair<bool, size_t>& loopingValue = TextFileUtils::readElementBool(lineBuffer, "looping", durationValue.second);

				std::vector<InnerTexture> innerTextures;
				//anim.InnerTextures.reserve(textureSequenceValue.first.size());
				for (const std::string& texture : textureSequenceValue.first) {
					innerTextures.emplace_back(atlasInfoFileData->InnerTextures.find(texture)->second);
				}

				TextureAtlasAnimation anim = { innerTextures, static_cast<float>(durationValue.first), loopingValue.first };
				atlasInfoFileData->Animations.emplace(animNameValue.first, anim);

				currentIndex += lineBuffer.size();
			}
		}

		return atlasInfoFileData;
	}
}
