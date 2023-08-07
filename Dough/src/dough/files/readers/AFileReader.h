#pragma once

#include "dough/Core.h"

#include "dough/files/FntFileData.h"

namespace DOH {

	/**
	* Assumes file is made up of chars.
	*
	* Elements are assumed to follow certain rules.
	* All elements have a Name or Label and an associated Value.
	* An element's name and value are always separated by '=' (note that there is no whitespace) and any type specific decoration.
	* Strings are decorated by '"' speach marks/double quotes on either side.
	*	e.g. String: characterName="James"
	* Single values of a non-string type start after the element's name's '=' and end with the next whitespace or a new line.
	*	e.g. Single values of a non-string type: maxHealth=200 
	* Vectors are stored inline with inner values being separated by a comma ',' (note that there is no whitespace) and ending with the first whitespace or a new line.
	*	e.g. Vector: pos=14.4,83.2
	* 
	* TODO:: Binary file support
	*/
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


		//
		// TODO:: More generic parsing functions, save re-writing mostly the same code per file type.
		//	Maybe have these be virtual functions so children class can alter them if needed?
		//
	};
}
