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
	};
}
