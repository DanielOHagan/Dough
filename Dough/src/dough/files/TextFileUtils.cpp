#include "dough/files/TextFileUtils.h"

#include "dough/Logging.h"

#include <tracy/public/tracy/Tracy.hpp>

namespace DOH {

	const std::pair<std::string, size_t> TextFileUtils::readElementString(
		const std::string& lineBuffer,
		const char* elementName,
		size_t startIndex
	) {
		ZoneScoped;

		const size_t elementNameLength = strlen(elementName);

		if (elementNameLength == 0) {
			LOG_ERR("Invalid search argument for readElementString. elementName: " << elementName);
			return { "", startIndex };
		}

		//Values will be truncated at MAX_STRING_CHAR_LENGTH - 1, leaving room for zero-termination char
		char valueChars[TextFileUtils::MAX_STRING_CHAR_LENGTH] = {};

		size_t targetElementStartIndex = lineBuffer.find(elementName, startIndex);
		if (targetElementStartIndex == std::string::npos) {
			LOG_ERR("Failed to find element: " << elementName);
			return { "", startIndex };
		}

		targetElementStartIndex += elementNameLength + 2; //Include offset of element name, '=' and first '"'
		const size_t targetElementEndIndex = lineBuffer.find("\"", targetElementStartIndex);
		size_t valueCharCount = targetElementEndIndex - targetElementStartIndex;

		if (valueCharCount >= TextFileUtils::MAX_STRING_CHAR_LENGTH) {
			LOG_WARN(
				"Length of string element: " << valueCharCount <<
				" must be smaller than: " << TextFileUtils::MAX_STRING_CHAR_LENGTH << " truncating to accommodate."
			);

			valueCharCount = static_cast<size_t>(TextFileUtils::MAX_STRING_CHAR_LENGTH - 1);
		}

		memcpy(
			&valueChars,
			lineBuffer.data() + targetElementStartIndex,
			sizeof(char) * valueCharCount
		);

		valueChars[valueCharCount] = '\0';

		return { valueChars, targetElementEndIndex }; //TODO:: targetElementEndIndex + 1? To account for the 2nd speach marks
	}

	const std::pair<std::vector<std::string>, size_t> TextFileUtils::readElementStringList(
		const std::string& lineBuffer,
		const char* elementName,
		size_t startIndex,
		uint32_t valueCount
	) {
		ZoneScoped;

		const std::pair<std::string, size_t>& listElementValue = TextFileUtils::readElementString(lineBuffer, elementName, startIndex);

		std::vector<std::string> innerStrings = {};

		size_t valueStartIndex = 0;
		size_t valueEndIndex = 0;
		for (uint32_t valuesRead = 0; valuesRead < valueCount; valuesRead++) {
			valueStartIndex = listElementValue.first.find_first_not_of(" ,\"", std::max(valueEndIndex, size_t(0)));

			if (valueStartIndex == std::string::npos) {
				if (valuesRead != valueCount) {
					LOG_WARN("readElementStringList did not read the expected number of strings. Expected: " << valueCount << " Read: " << valuesRead);
				}
				break;
			}
			valueEndIndex = listElementValue.first.find_first_of(",", valueStartIndex);

			bool finalValue = false;
			if (valueEndIndex == std::string::npos) {
				valueEndIndex = listElementValue.first.length();
				if (valueEndIndex <= valueStartIndex) {
					LOG_ERR("readElementStringList Failed to read final inner string value.");
					break;
				}

				finalValue = true;
			}

			innerStrings.emplace_back(listElementValue.first.substr(valueStartIndex, valueEndIndex - valueStartIndex));

			if (finalValue) {
				break;
			}
		}

		return { innerStrings, listElementValue.second };
	}

	const std::pair<int, size_t> TextFileUtils::readElementInt(
		const std::string& lineBuffer,
		const char* elementName,
		size_t startIndex
	) {
		ZoneScoped;

		const size_t elementNameLength = strlen(elementName);

		if (elementNameLength == 0) {
			LOG_ERR("Invalid search argument for readElementInt. elementName: " << elementName);
			return { INT16_MIN, startIndex };
		}

		//Values will be truncated at MAX_INT_CHAR_LENGTH - 1 digits, leaving room for zero-termination char
		char valueChars[TextFileUtils::MAX_INT_CHAR_LENGTH] = {};

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

		if (valueCharCount >= TextFileUtils::MAX_INT_CHAR_LENGTH) {
			LOG_WARN(
				"int valueCharCount of: " << valueCharCount <<
				" must be smaller than: " << TextFileUtils::MAX_INT_CHAR_LENGTH
			);
			//Truncate to ensure space for zero-termination character
			valueCharCount = static_cast<size_t>(TextFileUtils::MAX_INT_CHAR_LENGTH - 1);
		}
		valueChars[valueCharCount] = '\0';

		memcpy(
			&valueChars,
			lineBuffer.data() + targetElementStartIndex,
			sizeof(char) * valueCharCount
		);

		return { std::stoi(valueChars), targetElementStartIndex };
	}

	const std::pair<std::array<int, TextFileUtils::MAX_MULTIPLE_INT_COUNT>, size_t> TextFileUtils::readElementIntVector(
		const std::string& lineBuffer,
		const char* elementName,
		size_t startIndex,
		uint32_t valueCount
	) {
		ZoneScoped;

		const size_t elementNameLength = strlen(elementName);

		if (valueCount > TextFileUtils::MAX_MULTIPLE_INT_COUNT || valueCount == 0 || elementNameLength == 0) {
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
		int values[TextFileUtils::MAX_MULTIPLE_INT_COUNT] = {};
		bool finalValue = false;
		for (uint32_t i = 0; i < valueCount; i++) {
			if (finalValue) {
				LOG_WARN("Found end of vector before given valueCount reached. Found: " << i << " of desired: " << valueCount);
				break;
			}

			char valueChars[TextFileUtils::MAX_INT_CHAR_LENGTH] = {};
			targetElementEndIndex = lineBuffer.find_first_not_of("0123456789", targetElementStartIndex) + 1;
			size_t valueCharCount = targetElementEndIndex - targetElementStartIndex;

			if (valueCharCount >= TextFileUtils::MAX_FLOAT_CHAR_LENGTH) {
				LOG_WARN(
					"readElementIntVector. valueCharCount: " << valueCharCount <<
					" must be smaller than max: " << TextFileUtils::MAX_INT_CHAR_LENGTH
				);
				valueCharCount = static_cast<size_t>(TextFileUtils::MAX_INT_CHAR_LENGTH - 1);
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

	const std::pair<float, size_t> TextFileUtils::readElementFloat(
		const std::string& lineBuffer,
		const char* elementName,
		size_t startIndex
	) {
		ZoneScoped;

		const size_t elementNameLength = strlen(elementName);

		if (elementNameLength == 0) {
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

		if (valueCharCount >= TextFileUtils::MAX_INT_CHAR_LENGTH) {
			LOG_WARN(
				"int valueCharCount of: " << valueCharCount <<
				" must be smaller than: " << TextFileUtils::MAX_INT_CHAR_LENGTH
			);
			//Truncate to ensure space for zero-termination character
			valueCharCount = static_cast<size_t>(TextFileUtils::MAX_INT_CHAR_LENGTH - 1);
		}

		char valueChars[TextFileUtils::MAX_FLOAT_CHAR_LENGTH] = {};

		memcpy(
			&valueChars,
			lineBuffer.data() + targetElementStartIndex,
			sizeof(char) * valueCharCount
		);
		valueChars[valueCharCount] = '\0';

		return { std::stof(valueChars), targetElementStartIndex };
	}

	const std::pair<std::array<float, TextFileUtils::MAX_MULTIPLE_FLOAT_COUNT>, size_t> TextFileUtils::readElementFloatVector(
		const std::string& lineBuffer,
		const char* elementName,
		size_t startIndex,
		uint32_t valueCount
	) {
		ZoneScoped;

		const size_t elementNameLength = strlen(elementName);

		if (valueCount > TextFileUtils::MAX_MULTIPLE_FLOAT_COUNT || valueCount == 0 || elementNameLength == 0) {
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
		float values[TextFileUtils::MAX_FLOAT_CHAR_LENGTH] = {};
		bool finalValue = false;
		for (uint32_t i = 0; i < valueCount; i++) {
			if (finalValue) {
				LOG_WARN("Found end of vector before given valueCount reached. Found: " << i << " of desired: " << valueCount);
				break;
			}

			char valueChars[TextFileUtils::MAX_FLOAT_CHAR_LENGTH] = {};
			targetElementEndIndex = lineBuffer.find_first_not_of("-.0123456789", targetElementStartIndex) + 1;
			size_t valueCharCount = targetElementEndIndex - targetElementStartIndex;

			if (valueCharCount >= TextFileUtils::MAX_FLOAT_CHAR_LENGTH) {
				LOG_WARN(
					"readElementFloatVector. valueCharCount: " << valueCharCount <<
					" must be smaller than max: " << TextFileUtils::MAX_FLOAT_CHAR_LENGTH
				);
				valueCharCount = static_cast<size_t>(TextFileUtils::MAX_FLOAT_CHAR_LENGTH - 1);
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

	const std::pair<bool, size_t> TextFileUtils::readElementBool(
		const std::string& lineBuffer,
		const char* elementName,
		size_t startIndex
	) {
		ZoneScoped;

		const size_t elementNameLength = strlen(elementName);

		if (elementNameLength == 0) {
			LOG_ERR("Invalid search arguments for readElementBool. elementName is empty.");
			return { false , startIndex };
		}

		size_t targetElementStartIndex = lineBuffer.find(elementName, startIndex);
		if (targetElementStartIndex == std::string::npos) {
			LOG_ERR("readElementBool failed to find element: " << elementName);
			return { false, startIndex };
		}
		targetElementStartIndex += elementNameLength + 1; //Include + 1 for the '='

		switch (lineBuffer[targetElementStartIndex]) {
			case 'y':
			case 'Y':
				return { true, targetElementStartIndex + 1 };

			case 'n':
			case 'N':
				return { false, targetElementStartIndex + 1 };

			default:
				LOG_WARN("readElementBool element: " << elementName << " does not contain a value.");
				return { false, startIndex };
		}
	}
}
