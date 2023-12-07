#include "dough/files/readers/JsonFileReader.h"

#include "dough/files/ResourceHandler.h"
#include "dough/Logging.h"

#include <stack>

namespace DOH {

	JsonFileReader::JsonFileReader(const char* jsonFilePath, const bool openNow)
	:	AFileReader(jsonFilePath)
	{
		bool isValid = true;
		if (!ResourceHandler::isFileOfType(jsonFilePath, ".json")) {
			LOG_ERR("Invalid file type for reader (.json): " << jsonFilePath);
			isValid = false;
		}

		if (isValid && openNow) {
			if (!open()) {
				LOG_ERR("Failed to open file: " << jsonFilePath);
			}
		}
	}

	const bool JsonFileReader::open() {
		if (!mOpen) {
			mChars = ResourceHandler::readFile(mFilepath);
			mOpen = true;
		} else {
			LOG_WARN("Attempting to open an already open reader");
		}

		return mOpen;
	}

	std::shared_ptr<JsonFileData> JsonFileReader::read(const bool closeWhenRead) {
		if (!mOpen) {
			LOG_ERR("Attempting to read JSON file when not open");
			return nullptr;
		}

		JsonElement root = createElementObject();

		std::stack<std::reference_wrapper<JsonElement>> elementScope = {};
		std::reference_wrapper<JsonElement> currentElement = root;
		//Initialise to JSON_ROOT_OBJECT_NAME so first Object created is identified as the root object.
		std::string lastElementName = JSON_ROOT_OBJECT_NAME;
		//Init to a temp string which will only appear in malformed JSON and is an indication of broken JSON, this string should NOT end up being used!!
		std::string lastString = "temp_lastString";
		size_t stringStartIndex = 0;
		bool isInString = false;
		bool isSearchingForElementName = false;
		//Initialise to true because JSON starts and ends with { }, in the reader this is treated as an OBJECT.
		bool isSearchingForElementValue = true;
		bool isInNumber = false;
		//Initialise to true because this is only used when isInString is false and a number is found, assumed to be an integer unless a decimal point is found.
		bool isInteger = true;

		//Current line
		uint32_t debugLineNumber = 1;
		//Current index of char into current line
		uint32_t debugLineChar = 0;

		for (size_t i = 0; i < mChars.size(); i++) {
			const char c = mChars[i];

			switch (c) {
				case ' ':
				case '\t':
				case '\r':
				case '\n':
					if (c == '\n') {
						debugLineNumber++;
						debugLineChar = 0;
					}
					break;

				//Element names & String data values
				case '"':
					if (isInString) {
						isInString = false;

						const size_t stringSize = i - stringStartIndex;
						lastString = "";
						lastString.reserve(stringSize);
						for (size_t stringI = 0; stringI < stringSize; stringI++) {
							lastString.push_back(mChars[stringStartIndex + stringI]);
						}

						const bool isArray = currentElement.get().isArray();

						if (isSearchingForElementName) {
							isSearchingForElementValue = true;
							isSearchingForElementName = false;

							lastElementName = lastString;
						} else if (isSearchingForElementValue) {
							JsonElement element = createElementString(lastString);
							if (isArray) {
								currentElement.get().getArray().push_back(element);

								isSearchingForElementValue = true;
								isSearchingForElementName = false;
							} else {
								currentElement.get().getObject().emplace(lastElementName, element);

								isSearchingForElementValue = false;
								isSearchingForElementName = true;
							}
						}
					} else {
						stringStartIndex = i + 1; //Account for starting "
						isInString = true;
					}
					break;

				case '.':
					if (isInString) {
						continue;
					} else if (isInNumber) {
						isInteger = false;
					}
					continue;
				case '-':
					if (isInString) {
						continue;
					} else if (isSearchingForElementValue) {
						//Negative numbers
						stringStartIndex = i;
						isInNumber = true;
						isSearchingForElementValue = false;
					}
					continue;

				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					if (isInString) {
						break;
					} else if (isSearchingForElementValue) {
						isInNumber = true;

						stringStartIndex = i;
						isSearchingForElementValue = false;
					}

					break;

				case ',':
					if (isInString) {
						continue;
					} else if (isInNumber) {
						isInNumber = false;

						const size_t stringSize = i - stringStartIndex;
						lastString = "";
						lastString.reserve(stringSize);
						for (size_t stringI = 0; stringI < stringSize; stringI++) {
							lastString.push_back(mChars[stringStartIndex + stringI]);
						}

						if (isInteger) {
							JsonElement element = createElementLong(std::stol(lastString));
							if (currentElement.get().isArray()) {
								currentElement.get().getArray().push_back(element);

								isSearchingForElementValue = true;
								isSearchingForElementName = false;
							} else {
								currentElement.get().getObject().emplace(lastElementName, element);

								isSearchingForElementValue = false;
								isSearchingForElementName = true;
							}
						} else {
							JsonElement element = createElementDouble(std::stod(lastString));
							if (currentElement.get().isArray()) {
								currentElement.get().getArray().push_back(element);

								isSearchingForElementValue = true;
								isSearchingForElementName = false;
							} else {
								currentElement.get().getObject().emplace(lastElementName, element);

								isSearchingForElementValue = false;
								isSearchingForElementName = true;
							}

							//Reset to default value
							isInteger = true;
						}
					}
					break;

				case '{':
					if (isInString) {
						continue;
					} else {
						JsonElement element = createElementObject();

						const bool isArray = currentElement.get().isArray();
						if (isArray) {
							auto& arr = currentElement.get().getArray();
							arr.push_back(element);
							auto& latestElement = arr[arr.size() - 1];

							elementScope.push(latestElement);
							currentElement = latestElement;
						} else {
							auto& obj = currentElement.get().getObject();
							auto latestElement = obj.emplace(lastElementName, element);
							elementScope.push(latestElement.first->second);
							currentElement = latestElement.first->second;
						}

						isSearchingForElementValue = false;
						isSearchingForElementName = true;
					}
					break;
				case '}':
					if (isInString) {
						continue;
					} else {
						if (isInNumber) {
							isInNumber = false;

							const size_t stringSize = i - stringStartIndex;
							lastString = "";
							lastString.reserve(stringSize);
							for (size_t stringI = 0; stringI < stringSize; stringI++) {
								lastString.push_back(mChars[stringStartIndex + stringI]);
							}

							if (isInteger) {
								JsonElement element = createElementLong(std::stol(lastString));

								if (currentElement.get().isArray()) {
									currentElement.get().getArray().push_back(element);

									isSearchingForElementValue = true;
									isSearchingForElementName = false;
								} else {
									currentElement.get().getObject().emplace(lastElementName, element);

									isSearchingForElementValue = false;
									isSearchingForElementName = true;
								}
							} else {
								JsonElement element = createElementDouble(std::stod(lastString));

								if (currentElement.get().isArray()) {
									currentElement.get().getArray().push_back(element);

									isSearchingForElementValue = true;
									isSearchingForElementName = false;
								} else {
									currentElement.get().getObject().emplace(lastElementName, element);

									isSearchingForElementValue = false;
									isSearchingForElementName = true;
								}

								//Reset to default value
								isInteger = true;
							}

							if (!currentElement.get().isArray()) {
								isSearchingForElementName = true;
							}
						}

						elementScope.pop();
						if (elementScope.size() > 0) {
							currentElement = elementScope.top();
						}
					}
					break;

				//Array
				case '[':
					if (isInString) {
						continue;
					} else if (isSearchingForElementValue) {
						JsonElement element = createElementArray();

						if (currentElement.get().isArray()) {
							auto& arr = currentElement.get().getArray();
							arr.push_back(element);
							auto& latestElement = arr[arr.size() - 1];
							elementScope.push(latestElement);
							currentElement = latestElement;
						} else {
							auto& obj = currentElement.get().getObject();
							obj.emplace(lastElementName, element);
							auto latestElement = obj.emplace(lastElementName, element);
							elementScope.push(latestElement.first->second);
							currentElement = latestElement.first->second;
						}

						isSearchingForElementValue = true;
						isSearchingForElementName = false;
					}
					break;
				case ']':
					if (isInString) {
						continue;
					} else if (isInNumber) {
						isInNumber = false;

						const size_t stringSize = i - stringStartIndex;
						lastString = "";
						lastString.reserve(stringSize);
						for (size_t stringI = 0; stringI < stringSize; stringI++) {
							lastString.push_back(mChars[stringStartIndex + stringI]);
						}

						if (isInteger) {
							JsonElement element = createElementLong(std::stol(lastString));
							if (currentElement.get().isArray()) {
								currentElement.get().getArray().push_back(element);

								isSearchingForElementValue = true;
								isSearchingForElementName = false;
							} else {
								currentElement.get().getObject().emplace(lastElementName, element);

								isSearchingForElementValue = false;
								isSearchingForElementName = true;
							}
						} else {
							//TODO:: floating point precision issues
							double e = std::stod(lastString);
							JsonElement element = createElementDouble(std::stod(lastString));
							if (currentElement.get().isArray()) {
								currentElement.get().getArray().push_back(element);

								isSearchingForElementValue = true;
								isSearchingForElementName = false;
							} else {
								currentElement.get().getObject().emplace(lastElementName, element);

								isSearchingForElementValue = false;
								isSearchingForElementName = true;
							}

							//Reset to default value
							isInteger = true;
						}
					} else {
						elementScope.pop();
						if (elementScope.size() > 0) {
							currentElement = elementScope.top();
						}

						if (!currentElement.get().isArray()) {
							isSearchingForElementValue = false;
							isSearchingForElementName = true;
						}
					}
					break;

				//Null check
				case 'n':
					if (isInString) {
						continue;
					}

					if (mChars[i + 1] == 'u' && mChars[i + 2] == 'l' && mChars[i + 3] == 'l') {
						if (isSearchingForElementValue) {
							JsonElement element = createElementNull();

							if (currentElement.get().isArray()) {
								currentElement.get().getArray().push_back(element);

								isSearchingForElementValue = true;
								isSearchingForElementName = false;
							} else {
								currentElement.get().getObject().emplace(lastElementName, element);

								isSearchingForElementValue = false;
								isSearchingForElementName = true;
							}

							i += 2; //Skip ahead 2 chars + 1 for loop iteration because the next 3 characters are already known.
							break;

						} else if (isSearchingForElementName) {
							//This is when the element as a whole is null, not just the value.
							JsonElement element = createElementNull();
							if (currentElement.get().isArray()) {
								currentElement.get().getArray().push_back(element);

								isSearchingForElementValue = true;
								isSearchingForElementName = false;
							} else {
								currentElement.get().getObject().emplace(lastElementName, element);

								isSearchingForElementValue = false;
								isSearchingForElementName = true;
							}

							i += 2; //Skip ahead 2 chars + 1 for loop iteration because the next 3 characters are already known.
							break;
						}
					}
					break;

				//Booleans
				case 't':
					if (isInString) {
						continue;
					} else {
						if (mChars[i + 1] == 'r' && mChars[i + 2] == 'u' && mChars[i + 3] == 'e') {
							JsonElement element = createElementBool(true);

							if (currentElement.get().isArray()) {
								currentElement.get().getArray().push_back(element);

								isSearchingForElementValue = true;
								isSearchingForElementName = false;
							} else {
								currentElement.get().getObject().emplace(lastElementName, element);

								isSearchingForElementValue = false;
								isSearchingForElementName = true;
							}

							i += 2; //Skip ahead 2 chars + 1 for loop iteration because the next 3 characters are already known.
							break;
						} else {
							LOG_ERR("Assumed to find true but didn't. This likely affected the outcome of the program so data shouldn't be used! Malformed JSON line: " << debugLineNumber);
							break;
						}
					}
					break;
				case 'f':
					if (isInString) {
						continue;
					} else {
						if (mChars[i + 1] == 'a' && mChars[i + 2] == 'l' && mChars[i + 3] == 's' && mChars[i + 4] == 'e') {
							JsonElement element = createElementBool(false);

							if (currentElement.get().isArray()) {
								currentElement.get().getArray().push_back(element);

								isSearchingForElementValue = true;
								isSearchingForElementName = false;
							} else {
								currentElement.get().getObject().emplace(lastElementName, element);

								isSearchingForElementValue = false;
								isSearchingForElementName = true;
							}

							i += 3; //Skip ahead 3 chars + 1 for loop iteration because the next 3 characters are already known.
						} else {
							LOG_ERR("Assumed to find true but didn't. This likely affected the outcome of the program so data shouldn't be used! Malformed JSON line: " << debugLineNumber);
						}
					}
					break;
			}

			debugLineChar++;
		}

		if (root.Type != EJsonElementType::OBJECT) {
			LOG_ERR("Root object not created when reading JSON file: " << mFilepath);
			return nullptr;
		}

		std::shared_ptr<JsonFileData> fileData = std::make_shared<JsonFileData>();
		fileData->FileData = root.getObject();

		return fileData;
	}
}
