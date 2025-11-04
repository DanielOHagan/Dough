#include "dough/files/writers/JsonFileWriter.h"

#include "dough/files/JsonFileData.h"

#include <tracy/public/tracy/Tracy.hpp>

#include <fstream>

namespace DOH {

	JsonFileWriter::JsonFileWriter(const char* filePath)
	:	mFilePath(filePath)
	{}

	void JsonFileWriter::appendObjectCompact(JsonElement& element, std::stringstream& strStream) {
		if (element.Type != EJsonElementType::OBJECT) {
			LOG_ERR("appendObjectCompact given non-object element, given: " << EJsonElementTypeStrings[static_cast<uint32_t>(element.Type)]);
			return;
		}

		strStream << "{";

		std::unordered_map<std::string, JsonElement>& object = element.getObject();
		uint32_t currentElementIndex = 0;
		uint32_t objectChildCount = static_cast<uint32_t>(object.size());

		for (std::pair<std::string, JsonElement> entry : object) {
			switch (entry.second.Type) {
				case EJsonElementType::DATA_LONG:
					strStream << "\"" << entry.first << "\":" << entry.second.getLong();
					break;
				case EJsonElementType::DATA_DOUBLE:
					strStream << "\"" << entry.first << "\":" << entry.second.getDouble();
					break;
				case EJsonElementType::DATA_BOOL:
					strStream << "\"" << entry.first << "\":" << (entry.second.getBool() ? "true" : "false");
					break;
				case EJsonElementType::DATA_STRING:
					strStream << "\"" << entry.first << "\":\"" << entry.second.getString() << "\"";
					break;
				case EJsonElementType::OBJECT:
					strStream << "\"" << entry.first << "\":";
					appendObjectCompact(entry.second, strStream);
					break;
				case EJsonElementType::ARRAY:
					strStream << "\"" << entry.first << "\":";
					appendArrayCompact(entry.second, strStream);
					break;

				case EJsonElementType::NONE:
				default:
					break;
			}

			if (currentElementIndex < objectChildCount - 1) {
				strStream << ",";
			}

			currentElementIndex++;
		}

		strStream << '}';
	}

	void JsonFileWriter::appendArrayCompact(JsonElement& element, std::stringstream& strStream) {
		if (element.Type != EJsonElementType::ARRAY) {
			LOG_ERR("appendArrayCompact given non-array element, given: " << EJsonElementTypeStrings[static_cast<uint32_t>(element.Type)]);
			return;
		}

		strStream << "[";

		std::vector<JsonElement>& array = element.getArray();
		uint32_t currentElementIndex = 0;
		uint32_t arraySize = static_cast<uint32_t>(array.size());

		for (JsonElement& element : array) {
			switch (element.Type) {
				case EJsonElementType::DATA_LONG:
					strStream << element.getLong();
					break;
				case EJsonElementType::DATA_DOUBLE:
					strStream << element.getDouble();
					break;
				case EJsonElementType::DATA_BOOL:
					strStream << (element.getBool() ? "true" : "false");
					break;
				case EJsonElementType::DATA_STRING:
					strStream << "\"" << element.getString() << "\"";
					break;
				case EJsonElementType::OBJECT:
					appendObjectCompact(element, strStream);
					break;
				case EJsonElementType::ARRAY:
					appendArrayCompact(element, strStream);
					break;

				case EJsonElementType::NONE:
				default:
					break;
			}

			if (currentElementIndex < arraySize - 1 ) {
				strStream << ",";
			}

			currentElementIndex++;
		}

		strStream << ']';
	}

	bool JsonFileWriter::writeCompact(JsonFileData& data) {
		ZoneScoped;

		std::stringstream strStream = {};

		auto rootConditional = data.FileData.find(JSON_ROOT_OBJECT_NAME);
		if (rootConditional == data.FileData.end()) {
			LOG_ERR("JsonFileWriter::write failed to find root of: " << mFilePath);
			return false;
		}

		strStream << "{";
		
		//ROOT is handled separately because while ROOT is an object, its name isn't written into the JSON file like other objects.
		JsonElement& root = rootConditional->second;
		uint32_t currentElementIndex = 0;
		uint32_t objectChildCount = static_cast<uint32_t>(root.getObject().size());

		for (std::pair<std::string, JsonElement> entry : root.getObject()) {
			switch (entry.second.Type) {
				case EJsonElementType::DATA_LONG:
					strStream << "\"" << entry.first << "\":" << entry.second.getLong();
					break;
				case EJsonElementType::DATA_DOUBLE:
					strStream << "\"" << entry.first << "\":" << entry.second.getDouble();
					break;
				case EJsonElementType::DATA_BOOL:
					strStream << "\"" << entry.first << "\":" << (entry.second.getBool() ? "true" : "false");
					break;
				case EJsonElementType::DATA_STRING:
					strStream << "\"" << entry.first << "\":\"" << entry.second.getString() << "\"";
					break;
				case EJsonElementType::OBJECT:
					strStream << "\"" << entry.first << "\":";
					appendObjectCompact(entry.second, strStream);
					break;
				case EJsonElementType::ARRAY:
					strStream << "\"" << entry.first << "\":";
					appendArrayCompact(entry.second, strStream);
					break;

				case EJsonElementType::NONE:
				default:
					break;
			}

			if (currentElementIndex < objectChildCount - 1) {
				strStream << ",";
			}

			currentElementIndex++;
		}

		strStream << "}\n";

		std::ofstream outFile(mFilePath);
		if (outFile.is_open()) {
			outFile << strStream.rdbuf();

			outFile.close();
		} else {
			LOG_ERR("JsonFileWriter::write failed to open output file: " << mFilePath);
			return false;
		}

		return true;
	}
}
