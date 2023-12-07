#pragma once

#include "dough/files/readers/AFileReader.h"
#include "dough/files/JsonFileData.h"

namespace DOH {

	class JsonFileReader : public AFileReader<JsonFileData> {
	public:
		JsonFileReader(const char* jsonFilePath, const bool openNow = true);
		JsonFileReader(const JsonFileReader& copy) = delete;
		JsonFileReader operator=(const JsonFileReader& assignment) = delete;

		virtual const bool open() override;
		virtual std::shared_ptr<JsonFileData> read(const bool closeWhenRead = true) override;

		inline const bool isOpen() const { return mOpen; }
		inline void close() { mChars.clear(); mOpen = false; }
	};
}
