#pragma once

#include "dough/Core.h"
#include "dough/files/FntFileData.h"
#include "dough/files/readers/AFileReader.h"

namespace DOH {

	class FntFileReader : public AFileReader<FntFileData> {

	public:
		FntFileReader(const char* filepath, const bool openNow = true);
		FntFileReader(const FntFileReader& copy) = delete;
		FntFileReader operator=(const FntFileReader& assignment) = delete;

		virtual const bool open() override;
		virtual std::shared_ptr<FntFileData> read(const bool closeWhenRead = true) override;

		inline const bool isOpen() const { return mOpen; }
		inline void close() { mChars.clear(); mOpen = false; }

	private:
		//returns pair<parsed int value, offset of final char in value>
		static std::pair<int, size_t> readFntFileElementInt(
			const std::string& lineBuffer,
			const char* elementName,
			const size_t startIndex = 0
		);
		static std::pair<std::array<int, 4>, size_t> readFntFileElementMultipleInt(
			const std::string& lineBuffer,
			const char* elementName,
			const uint32_t intCount,
			const size_t startIndex
		);
		//returns <read string value, offset of final char in value>
		static std::pair<std::string, size_t> readFntFileElementString(
			const std::string& lineBuffer,
			const char* elementName,
			const size_t startIndex = 0
		);
	};
}
