#pragma once

#include "dough/files/AFileData.h"
#include "dough/Core.h"
#include "dough/Logging.h"

#include <variant>

namespace DOH {

	//Used to treat element as null 
	static constexpr long JSON_DEFAULT_LONG_VALUE = LONG_MAX;
	static constexpr const char* JSON_ROOT_OBJECT_NAME = "__ROOT__";

	static constexpr std::array<const char*, 7> EJsonElementTypeStrings = {
		"NONE",

		"DATA_LONG",
		"DATA_DOUBLE",
		"DATA_BOOL",
		"DATA_STRING",

		"OBJECT",
		"ARRAY"
	};

	enum class EJsonElementType {
		NONE,

		DATA_LONG,
		DATA_DOUBLE,
		DATA_BOOL,
		DATA_STRING,

		OBJECT,
		ARRAY
	};

	struct JsonElement {
		EJsonElementType Type;
		//If DATA____ then respective value data is stored (except for null, see below), if OBJECT then an unordered_map to child elements, if ARRAY then array of elements.
		//If element value is null then Type is set to EJsonElementType::NONE and an int of value
		//JSON_DEFAULT_LONG_VALUE (2147483647) (a.k.a LONG_MAX from limits.h) will be used stored.
		std::variant<
			long,											// Primitve Data:	EJsonElementType::DATA_LONG
			double,											// Primitve Data:	EJsonElementType::DATA_DOUBLE
			bool,											// Primitve Data:	EJsonElementType::DATA_BOOL
			std::string,									// Primitve Data:	EJsonElementType::DATA_STRING
			std::unordered_map<std::string, JsonElement>,	// Object:			EJsonElementType::OBJECT
			std::vector<JsonElement>						// Array:			EJsonElementType::ARRAY
		> Element;

		//Helper functions with type checks
		inline void setLong(long value) {
			if (Type == EJsonElementType::DATA_LONG) {
				Element.emplace<long>(value);
			} else {
				LOG_ERR("Attempted to set long value of non-long JsonElement");
			}
		};
		inline long getLong() const { return std::get<long>(Element); }

		inline void setDouble(double value) {
			if (Type == EJsonElementType::DATA_DOUBLE) {
				Element.emplace<double>(value);
			} else {
				LOG_ERR("Attempted to set double value of non-double JsonElement");
			}
		};
		inline double getDouble() const { return std::get<double>(Element); }
		inline double getNumberAsDouble() const {
			if (isDouble()) {
				return std::get<double>(Element);
			} else if (isLong()) {
				return std::get<long>(Element);
			} else {
				LOG_WARN("Attempted to get number when element is not a number.");
				return -1;
			}
		}
		inline float getNumberAsFloat() const { return static_cast<float>(getNumberAsDouble()); }

		inline void setBool(bool value) {
			if (Type == EJsonElementType::DATA_BOOL) {
				Element.emplace<bool>(value);
			} else {
				LOG_ERR("Attempted to set bool value of non-bool JsonElement");
			}
		};
		inline bool getBool() const { return std::get<bool>(Element); }

		inline void setString(const std::string& value) {
			if (Type == EJsonElementType::DATA_STRING) {
				Element.emplace<std::string>(value);
			} else {
				LOG_ERR("Attempted to set string value of non-string JsonElement");
			}
		};
		inline const std::string& getString() const { return std::get<std::string>(Element); }

		inline void setObject(const std::unordered_map<std::string, JsonElement>& value) {
			if (Type == EJsonElementType::OBJECT) {
				Element.emplace<std::unordered_map<std::string, JsonElement>>(value);
			} else {
				LOG_ERR("Attempted to set JsonObject value of non-JsonObject JsonElement");
			}
		};
		inline std::unordered_map<std::string, JsonElement>& getObject() { return std::get<std::unordered_map<std::string, JsonElement>>(Element); }

		inline void setArray(const std::vector<JsonElement>& value) {
			if (Type == EJsonElementType::ARRAY) {
				Element.emplace<std::vector<JsonElement>>(value);
			} else {
				LOG_ERR("Attempted to set JsonArray value of non-JsonArray JsonElement");
			}
		};
		//Non-const refernce to array
		inline std::vector<JsonElement>& getArray() { return std::get<std::vector<JsonElement>>(Element); }

		//Object element search
		inline JsonElement& operator[](const char* elementName) {
			if (Type == EJsonElementType::OBJECT) {
				std::unordered_map<std::string, JsonElement>& obj = getObject();
				const auto& result = obj.find(elementName);
				if (result != obj.end()) {
					return result->second;
				}
			}

			return *this;
		}

		//Object element search with safety wrapper
		inline std::optional<JsonElement> getElement(const char* elementName) {
			if (Type == EJsonElementType::OBJECT) {
				std::unordered_map<std::string, JsonElement>& obj = getObject();
				const auto& result = obj.find(elementName);
				if (result != obj.end()) {
					return { result->second };
				}
			}

			return {};
		}

		//Array index search
		inline JsonElement& operator[](const size_t index) {
			if (Type == EJsonElementType::ARRAY) {
				return getArray()[index];
			}
		}

		inline bool hasElement(const char* elementName) {
			if (Type == EJsonElementType::OBJECT) {
				return getObject().find(elementName) != getObject().end();
			} else {
				LOG_ERR("Attempted to find element: " << elementName << " on a non-object element.");
			}

			return false;
		}

		inline bool isNull() const { return Type == EJsonElementType::NONE; }
		inline bool isLong() const { return Type == EJsonElementType::DATA_LONG; }
		inline bool isDouble() const { return Type == EJsonElementType::DATA_DOUBLE; }
		inline bool isBool() const { return Type == EJsonElementType::DATA_BOOL; }
		inline bool isString() const { return Type == EJsonElementType::DATA_STRING; }
		inline bool isObject() const { return Type == EJsonElementType::OBJECT; }
		inline bool isArray() const { return Type == EJsonElementType::ARRAY; }
	};

	inline static JsonElement createElementNull() {
		JsonElement e = { EJsonElementType::NONE };
		e.Element = JSON_DEFAULT_LONG_VALUE;
		return e;
	}

	inline static JsonElement createElementLong(long value) {
		JsonElement e = { EJsonElementType::DATA_LONG };
		e.Element = value;
		return e;
	}

	inline static JsonElement createElementDouble(double value) {
		JsonElement e = { EJsonElementType::DATA_DOUBLE };
		e.Element = value;
		return e;
	}

	inline static JsonElement createElementBool(bool value) {
		JsonElement e = { EJsonElementType::DATA_BOOL };
		e.Element = value;
		return e;
	}

	inline static JsonElement createElementString(const std::string& value) {
		JsonElement e = { EJsonElementType::DATA_STRING };
		e.Element = value;
		return e;
	}

	inline static JsonElement createElementObject() {
		JsonElement e = { EJsonElementType::OBJECT };
		e.Element = std::unordered_map<std::string, JsonElement>();
		return e;
	}

	inline static JsonElement createElementArray() {
		JsonElement e = { EJsonElementType::ARRAY };
		e.Element = std::vector<JsonElement>();
		return e;
	}

	struct JsonFileData : public AFileData {
		//TODO:: Replace this is a JsonElement RootElementObject? Would make getting root easier and faster.
		std::unordered_map<std::string, JsonElement> FileData;

		//TODO:: FileData MUST have an entry at JSON_ROOT_OBJECT_NAME
		inline JsonElement& getRoot() { return FileData.at(JSON_ROOT_OBJECT_NAME); }
	};
}
