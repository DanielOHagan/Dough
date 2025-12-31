#include "dough/input/InputActionMap.h"

#include "dough/files/ResourceHandler.h"
#include "dough/Logging.h"
#include "dough/input/AInputLayer.h"
#include "dough/files/JsonFileData.h"
#include "dough/input/InputCodes.h"

namespace DOH {

	InputActionMap::InputActionMap()
	:	mActions({})
	{}

	InputActionMap::InputActionMap(const char* filePath)
	:	mActions({})
	{
		addActionsFromFile(filePath);
	}
	
	void InputActionMap::addActionsFromFile(const char* filePath) {
		if (ResourceHandler::isFileOfType(filePath, ".json")) {
			std::shared_ptr<JsonFileData> fileData = ResourceHandler::loadJsonFile(filePath);
			if (fileData == nullptr) {
				LOG_WARN("InputActionMap::loadActionsFromFile failed to load json data: " << filePath);
			}

			std::unordered_map<std::string, JsonElement>& root = fileData->getRoot().getObject();
			for (std::pair<std::string, JsonElement> actionEntry : root) {
				std::unordered_map<std::string, JsonElement>& actionData = actionEntry.second.getObject();
				InputAction action = {};
				uint32_t stepIndex = 0;
				//If no actions (for any device type) have been found.
				bool emptyAction = true;
				const auto& kbmEntry = actionData.find(InputAction::JSON_KEYBOARD_AND_MOUSE_STRING);
				if (kbmEntry != actionData.end()) {
					std::vector<JsonElement>& steps = kbmEntry->second.getArray();

					for (JsonElement& stepJsonElement : steps) {
						std::unordered_map<std::string, JsonElement>& step = stepJsonElement.getObject();
						JsonElement& typeElement = step.at(InputAction::JSON_ACTION_TYPE_STRING);
						EDeviceInputType type = EDeviceInputType::NONE;

						if (typeElement.isString()) {
							const std::string& typeString = typeElement.getString();
							type = InputAction::getEDeviceInputTypeFromString(typeString.c_str());
						} else if (typeElement.isLong()) {
							type = static_cast<EDeviceInputType>(typeElement.getLong());
						} else {
							LOG_ERR(
								"InputActionMap::addActionsFromFile action type represented by unsupported type. " << filePath <<
								"\t action: " << actionEntry.first << " t: " << typeElement.Element.index()
							);
							continue;
						}

						JsonElement& valueElement = step.at(InputAction::JSON_ACTION_VALUE_STRING);
						int value = -1;
						if (valueElement.isLong()) {
							value = static_cast<int>(valueElement.getLong());
						} else {
							LOG_ERR(
								"InputActionMap::addActionsFromFile action value represented by unsupported type. " << filePath <<
								"\t action: " << actionEntry.first << " v: " << valueElement.Element.index()
							);
							continue;
						}

						action.ActionCodesByDevice[stepIndex] = { type, value };
						stepIndex++;
					}

					if (stepIndex > 0) {
						mActions.emplace(actionEntry.first.c_str(), action);
						emptyAction = false;
					}
				}

				const auto& conEntry = actionData.find(InputAction::JSON_CONTROLLER_STRING);
				if (conEntry != actionData.end()) {
					//TODO:: Controller support.

					//TEMP:: Prevent multiple warnings from same file.
					static bool alreadyWarned = false;
					if (!alreadyWarned) {
						LOG_WARN(filePath << " uses controller inputs when NOT SUPPORTED!");
						alreadyWarned = true;
					}
				}

				if (emptyAction) {
					LOG_WARN("No inputs found for action: " << actionEntry.first << " - " << filePath);
				}
			}
	
		} else {
			LOG_ERR("InputActionMap::loadActionsFromFile given file NOT of type .json: " << filePath);
		}
	}

	bool InputActionMap::addAction(const char* name, InputAction& action) {
		auto result = mActions.find(name);
		if (result == mActions.end()) {
			//Ensure action has at least one possible input.
			if (action.ActionCodesByDevice[0].first == EDeviceInputType::NONE) {
				LOG_WARN("InputActionMap::addAction given action with EDeviceInputType::NONE first action: " << name);
				return false;
			}
			auto& r = mActions.emplace(name, action);
			return r.second;
		} else {
			LOG_WARN("InputActionMap::addAction failed to add action, name taken: " << name);
			return false;
		}
	}

	void InputActionMap::updateAction(const char* name, InputAction& action) {
		auto result = mActions.find(name);
		if (result != mActions.end()) {
			//Ensure action has at least one possible input.
			if (action.ActionCodesByDevice[0].first == EDeviceInputType::NONE) {
				LOG_WARN("InputActionMap::updateAction given action with EDeviceInputType::NONE first action: " << name);
			}
			result->second.ActionCodesByDevice = action.ActionCodesByDevice;
		} else {
			LOG_WARN("InputActionMap::updateAction action not found: " << name);
		}
	}

	void InputActionMap::removeAction(const char* name) {
		mActions.erase(name);
	}

	bool InputActionMap::hasAction(const char* name) {
		auto result = mActions.find(name);
		return result == mActions.end();
	}

	bool InputActionMap::isActionActiveAND(const char* name, const AInputLayer& inputLayer) const {
		auto& actionItr = mActions.find(name);
		if (actionItr == mActions.end()) {
			LOG_WARN("isActionActive action not found: " << name);
			return false;
		}
		return actionItr->second.isActiveAND(inputLayer);
	}

	bool InputActionMap::isActionActiveANDConsume(const char* name, AInputLayer& inputLayer) {
		auto& actionItr = mActions.find(name);
		if (actionItr == mActions.end()) {
			LOG_WARN("isActionActiveConsume action not found: " << name);
			return false;
		}
		return actionItr->second.isActiveANDConsume(inputLayer);
	}

	bool InputAction::isActiveAND(const AInputLayer& inputLayer) const {
		bool active = true;
		for (std::pair<EDeviceInputType, int> actionStep : ActionCodesByDevice) {
			switch (actionStep.first) {
				case EDeviceInputType::KEY_PRESS:
					active = active && inputLayer.isKeyPressed(actionStep.second);
					break;
				case EDeviceInputType::MOUSE_PRESS:
					active = active && inputLayer.isMouseButtonPressed(actionStep.second);
					break;
				case EDeviceInputType::MOUSE_SCROLL_DOWN:
					active = active && inputLayer.isMouseScrollingDown();
					break;
				case EDeviceInputType::MOUSE_SCROLL_UP:
					active = active && inputLayer.isMouseScrollingUp();
					break;
				case EDeviceInputType::MOUSE_MOVE:
					continue; //Mouse movement can by handled by "Input::getMousePos()"

					//NONE is used to show that no more actions are expected.
				default:
				case EDeviceInputType::NONE:
					return active;
			}
		}
		return active;
	}
	
	bool InputAction::isActiveANDConsume(AInputLayer& inputLayer) {
		bool active = true;
		for (std::pair<EDeviceInputType, int> actionStep : ActionCodesByDevice) {
			if (!active) break;
			bool stepActive = false;
			switch (actionStep.first) {
				case EDeviceInputType::KEY_PRESS:
					active = active && (stepActive = inputLayer.isKeyPressed(actionStep.second));
					break;
				case EDeviceInputType::MOUSE_PRESS:
					active = active && (stepActive = inputLayer.isMouseButtonPressed(actionStep.second));
					break;
				case EDeviceInputType::MOUSE_SCROLL_DOWN:
					active = active && (stepActive = inputLayer.isMouseScrollingDown());
					break;
				case EDeviceInputType::MOUSE_SCROLL_UP:
					active = active && (stepActive = inputLayer.isMouseScrollingUp());
					break;
				case EDeviceInputType::MOUSE_MOVE:
					continue; //Mouse movement can by handled by "Input::getMousePos()"

				//NONE is used to show that no more actions are expected.
				default:
				case EDeviceInputType::NONE:
					if (active) inputLayer.consumeAction(*this);
					return active;
			}
		}

		if (active) inputLayer.consumeAction(*this);
		return active;
	}

	EDeviceInputType InputAction::getEDeviceInputTypeFromString(const char* string) {
		for (int i = 1; i < EDeviceInputTypeCount; i++) {
			if (strcmp(string, EDeviceInputType_Strings[i]) == 0) {
				return static_cast<EDeviceInputType>(i);
			}
		}

		return EDeviceInputType::NONE;
	}
}
