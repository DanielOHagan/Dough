#pragma once

#include <sstream>

namespace DOH {

	struct JsonFileData;
	struct JsonElement;

	class JsonFileWriter { // : public AFileWriter<JsonFileData> {
	private:
		const char* mFilePath;

		void appendObjectCompact(JsonElement& element, std::stringstream& strStream);
		void appendArrayCompact(JsonElement& element, std::stringstream& strStream);

	public:
		JsonFileWriter(const char* filePath);
		JsonFileWriter(const JsonFileWriter& copy) = delete;
		void operator=(const JsonFileWriter& assignment) = delete;

		bool writeCompact(JsonFileData& data);

		inline const char* getFilePath() const { return mFilePath; }

	};
}
