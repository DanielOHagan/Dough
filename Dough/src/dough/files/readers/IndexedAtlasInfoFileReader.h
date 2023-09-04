#pragma once

#include "dough/files/readers/AFileReader.h"
#include "dough/files/IndexedAtlasInfoFileData.h"

namespace DOH {

	class IndexedAtlasInfoFileReader : public AFileReader<IndexedAtlasInfoFileData> {	
	public:
		IndexedAtlasInfoFileReader(const char* atlasInfoFilePath, const bool openNow = true);
		IndexedAtlasInfoFileReader(const IndexedAtlasInfoFileReader& copy) = delete;
		IndexedAtlasInfoFileReader operator=(const IndexedAtlasInfoFileReader& assignment) = delete;

		virtual const bool open() override;
		virtual std::shared_ptr<IndexedAtlasInfoFileData> read(const bool closeWhenRead = true) override;

		inline const bool isOpen() const { return mOpen; }
		inline void close() { mChars.clear(); mOpen = false; }
	};
}
