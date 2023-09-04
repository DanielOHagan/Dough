#pragma once

#include <cstdint>
#include <utility>
#include <string>
#include <array>
#include <vector>

namespace DOH {

	/**
	* 
	* Elements are assumed to follow certain rules.
	* All elements have a Name or Label and an associated Value.
	* An element's name and value are always separated by '=' (note that there is no whitespace) and any type specific decoration.
	* Strings are decorated by '"' speach marks/double quotes on either side.
	*	e.g. String: characterName="James"
	* Single values of a non-string type start after the element's name's '=' and end with the next whitespace or a new line.
	*	e.g. Single values of a non-string type: maxHealth=200
	* Vectors are stored inline with inner values being separated by a comma ',' (note that there is no whitespace) and ending with the first whitespace or a new line.
	* Vectors have a limited number of values as they are designed to replicate certain data types, for example a coordinate in 3D space.
	*	e.g. Vector: pos=14.4,83.2
	* Lists work very similarly to Vectors, however, they do not limit the number of inner values, due to this the results are stored in an std::vector.
	* List decorations depend on the data type, and so Lists will ONLY CONTAIN ONE type of data.
	* Booleans are treated as Single Values despite being a string. Booleans use the chars y and Y for a true value and the chars n and N for a false value.
	*	e.g. Boolean: isEnabled=y
	*
	*/

	class TextFileUtils {
	public:
		constexpr static uint32_t MAX_STRING_CHAR_LENGTH = 128;
		constexpr static uint32_t MAX_INT_CHAR_LENGTH = 8;
		constexpr static uint32_t MAX_FLOAT_CHAR_LENGTH = 8;
		constexpr static uint32_t MAX_MULTIPLE_INT_COUNT = 4;
		constexpr static uint32_t MAX_MULTIPLE_FLOAT_COUNT = 4;
		constexpr static uint32_t LIST_READ_COUNT_ALL = UINT32_MAX;

		/**
		* readElement___ and readElement___Vector read values of a given type from a single line.
		*
		* Each one returns the requested data type and an index of the last possible position the function read from, except if the function fails to read an element.
		* Upon a failed read the function returns a default value for the requested type and the index is the same as the given startIndex param used in every readElement function.
		*/

		/**
		* Search a string value for an element cotaining a string. That string being denoted by double qoutation marks '"'.
		*
		* @param lineBuffer The line to search for the given element.
		* @param elementName The requested element.
		* @param startIndex The index into the lineBuffer where to start. Defaults to 0.
		* @returns Pair containing the read value and the index at the end of the final index of the element.
		*/
		static const std::pair<std::string, size_t> readElementString(
			const std::string& lineBuffer,
			const char* elementName,
			size_t startIndex = 0
		);
		/**
		* Search a string value for an element containing a list of strings. The inner values are delimited by a comma ',' and decorated by double quotation marks '"' at the start and end of the list.
		* NOT for individual inner values.
		* 
		* @param lineBuffer The line to search for the given element.
		* @param elementName The requested element.
		* @param startIndex The index into the lineBuffer where to start. Defaults to 0.
		* @param valueCount The number of values to try to read. Defaults to 1.
		* @returns Pair containing the read value and the index at the end of the final index of the element.
		*/
		static const std::pair<std::vector<std::string>, size_t> readElementStringList(
			const std::string& lineBuffer,
			const char* elementName,
			size_t startIndex = 0,
			uint32_t valueCount = 1
		);
		/**
		* Search a string value for an element cotaining an integer. The integer is read as any whole signed/unsigned number until any non-numerical character is found.
		*
		* @param lineBuffer The line to search for the given element.
		* @param elementName The requested element.
		* @param startIndex The index into the lineBuffer where to start. Defaults to 0.
		* @returns Pair containing the read value and the index at the end of the final index of the element.
		*/
		static const std::pair<int, size_t> readElementInt(
			const std::string& lineBuffer,
			const char* elementName,
			size_t startIndex = 0
		);
		/**
		* Search a string value for an element containing multiple integer values. The integers are read as any whole signed/unsigned number any non-numberical character is found.
		* Individual integer values are separated by a comma ','.
		*
		* @param lineBuffer The line to search for the given element.
		* @param elementName The requested element.
		* @param startIndex The index into the lineBuffer where to start. Defaults to 0.
		* @param valueCount The number of values to try to read. Defaults to MAX_MULTIPLE_INT_COUNT (4).
		* @returns Pair containing the read value and the index at the end of the final index of the element.
		*/
		static const std::pair<std::array<int, TextFileUtils::MAX_MULTIPLE_INT_COUNT>, size_t> readElementIntVector(
			const std::string& lineBuffer,
			const char* elementName,
			size_t startIndex = 0,
			uint32_t valueCount = TextFileUtils::MAX_MULTIPLE_INT_COUNT
		);
		/**
		* Search a string value for an element cotaining a float. The float is read as any signed/unsigned number until any non-numerical character is found.
		*
		* @param lineBuffer The line to search for the given element.
		* @param elementName The requested element.
		* @param startIndex The index into the lineBuffer where to start. Defaults to 0.
		* @returns Pair containing the read value and the index at the end of the final index of the element.
		*/
		static const std::pair<float, size_t> readElementFloat(
			const std::string& lineBuffer,
			const char* elementName,
			size_t startIndex = 0
		);
		/**
		* Search a string value for an element containing multiple float values. The floats are read as any signed/unsigned number any non-numberical character is found.
		* Individual float values are separated by a comma ','.
		*
		* @param lineBuffer The line to search for the given element.
		* @param elementName The requested element.
		* @param startIndex The index into the lineBuffer where to start. Defaults to 0.
		* * @param valueCount The number of values to try to read. Defaults to MAX_MULTIPLE_FLOAT_COUNT (4).
		* @returns Pair containing the read value and the index at the end of the final index of the element.
		*/
		static const std::pair<std::array<float, TextFileUtils::MAX_MULTIPLE_FLOAT_COUNT>, size_t> readElementFloatVector(
			const std::string& lineBuffer,
			const char* elementName,
			size_t startIndex = 0,
			uint32_t valueCount = TextFileUtils::MAX_MULTIPLE_FLOAT_COUNT
		);
		/**
		* Search a string value for either the true values of: y, Y. Or the false values of: n, N.
		*
		* @param lineBuffer The line to search for the given element.
		* @param elementName The requested element.
		* @param startIndex The index into the lineBuffer where to start. Defaults to 0.
		* @returns Pair containing the read value and the index at the end of the final index of the element.
		*/
		static const std::pair<bool, size_t> readElementBool(
			const std::string& lineBuffer,
			const char* elementName,
			size_t startIndex = 0
		);
	};
}
