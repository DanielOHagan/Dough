#pragma once

#include "dough/files/readers/AFileReader.h"
#include "dough/files/IndexedAtlasInfoFileData.h"

namespace DOH {

	class IndexedAtlasInfoFileReader : public AFileReader<IndexedAtlasInfoFileData> {
	private:
		constexpr static uint32_t MAX_STRING_CHAR_LENGTH = 128;
		constexpr static uint32_t MAX_INT_CHAR_LENGTH = 8;
		constexpr static uint32_t MAX_FLOAT_CHAR_LENGTH = 8;
		constexpr static uint32_t MAX_MULTIPLE_INT_COUNT = 4;
		constexpr static uint32_t MAX_MULTIPLE_FLOAT_COUNT = 4;
	
	public:
		IndexedAtlasInfoFileReader(const char* atlasInfoFilePath, const bool openNow = true);
		IndexedAtlasInfoFileReader(const IndexedAtlasInfoFileReader& copy) = delete;
		IndexedAtlasInfoFileReader operator=(const IndexedAtlasInfoFileReader& assignment) = delete;

		virtual const bool open() override;
		virtual std::shared_ptr<IndexedAtlasInfoFileData> read(const bool closeWhenRead = true) override;

		inline const bool isOpen() const { return mOpen; }
		inline void close() { mChars.clear(); mOpen = false; }

		static const std::pair<std::string, size_t> readElementString(
			const std::string& lineBuffer,
			const char* elementName,
			size_t startIndex = 0
		);
		static const std::pair<int, size_t> readElementInt(
			const std::string& lineBuffer,
			const char* elementName,
			size_t startIndex = 0
		);
		static const std::pair<std::array<int, IndexedAtlasInfoFileReader::MAX_MULTIPLE_INT_COUNT>, size_t> readElementIntVector(
			const std::string& lineBuffer,
			const char* elementName,
			size_t startIndex = 0,
			uint32_t valueCount = IndexedAtlasInfoFileReader::MAX_MULTIPLE_FLOAT_COUNT
		);
		static const std::pair<float, size_t> readElementFloat(
			const std::string& lineBuffer,
			const char* elementName,
			size_t startIndex = 0
		);
		static const std::pair<std::array<float, IndexedAtlasInfoFileReader::MAX_MULTIPLE_FLOAT_COUNT>, size_t> readElementFloatVector(
			const std::string& lineBuffer,
			const char* elementName,
			size_t startIndex = 0,
			uint32_t valueCount = IndexedAtlasInfoFileReader::MAX_MULTIPLE_FLOAT_COUNT
		);
	};
}
