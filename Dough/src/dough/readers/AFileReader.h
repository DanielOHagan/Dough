#pragma once

#include "dough/Core.h"

#include "dough/model/FntFileData.h"

namespace DOH {

	template<typename T, typename = std::enable_if<std::is_base_of<AFileData, T>::value>>
	class AFileReader {
	protected:
		std::vector<char> mChars;
		const char* mFilepath;
		bool mOpen;

		AFileReader(const char* filepath)
		:	mFilepath(filepath),
			mOpen(false)
		{}

	public:
		AFileReader(const AFileReader& copy) = delete;
		AFileReader operator=(const AFileReader& assignment) = delete;

		virtual const bool open() = 0;
		virtual std::shared_ptr<T> read(const bool closeWhenRead) = 0;

		inline const bool isOpen() const { return mOpen; }
		inline void close() { mChars.clear(); mOpen = false; }
	};
}
