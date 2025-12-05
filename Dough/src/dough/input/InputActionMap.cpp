#include "dough/input/InputActionMap.h"

#include "dough/files/ResourceHandler.h"
#include "dough/Logging.h"
#include "dough/input/AInputLayer.h"

namespace DOH {

	InputActionMap::InputActionMap()
	:	mActions({})
	{}

	//InputActionMap::InputActionMap(const char* filePath)
	//:	mActions({})
	//{
	//	loadActionsFromFile(filePath);
	//}
	//
	//void InputActionMap::loadActionsFromFile(const char* filePath) {
	//	if (ResourceHandler::isFileOfType(filePath, ".json")) {
	//
	//
	//
	//	} else {
	//		LOG_ERR("InputActionMap::loadActionsFromFile given file NOT of type .json: " << filePath);
	//	}
	//}

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

	bool InputActionMap::isActionActive(const char* name, const AInputLayer& inputLayer) {
		//Assumes action exists.
		InputAction& action = mActions.at(name);
		bool active = true;
		for (std::pair<EDeviceInputType, int> actionStep : action.ActionCodesByDevice) {
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

	//TODO:: How much of an issue will this be? If an earlier check is true it consumes but a later one may be false so the action as a whole shouldn't consume.
	bool InputActionMap::isActionActiveConsume(const char* name, AInputLayer& inputLayer) {
		//Assumes action exists.
		InputAction& action = mActions.at(name);
		bool active = true;
		for (std::pair<EDeviceInputType, int> actionStep : action.ActionCodesByDevice) {
			switch (actionStep.first) {
				case EDeviceInputType::KEY_PRESS:
					active = active && inputLayer.isKeyPressedConsume(actionStep.second);
					break;
				case EDeviceInputType::MOUSE_PRESS:
					active = active && inputLayer.isMouseButtonPressedConsume(actionStep.second);
					break;
				case EDeviceInputType::MOUSE_SCROLL_DOWN:
					active = active && inputLayer.isMouseScrollingDownConsume();
					break;
				case EDeviceInputType::MOUSE_SCROLL_UP:
					active = active && inputLayer.isMouseScrollingUpConsume();
					break;
				case EDeviceInputType::MOUSE_MOVE:
					continue; //Mouse movement can by handled by "Input::getMousePos()"

					//NONE is used to show that no more actions are expected.
				default:
				case EDeviceInputType::NONE:
					return active;
					break;
			}
		}

		return active;
	}
}
