#include "dough/input/DeviceInput.h"

#include "dough/Logging.h"
#include "dough/input/Input.h"

namespace DOH {

	DeviceInputKeyboardMouse::DeviceInputKeyboardMouse()
	:	MouseScreenPos(0.0f, 0.0f),
		MouseScrollOffset(0.0f, 0.0f)
	{
		std::vector<int> keyCodes = {};
		keyCodes.reserve(Input::DEFAULT_KEY_CODES.size());
		std::vector<int> mouseButtons = {};
		mouseButtons.reserve(Input::DEFAULT_MOUSE_BUTTON_CODES.size());

		for (int keyCode : Input::DEFAULT_KEY_CODES) {
			keyCodes.emplace_back(keyCode);
		}

		for (int button : Input::DEFAULT_MOUSE_BUTTON_CODES) {
			mouseButtons.emplace_back(button);
		}

		setPossibleKeyInputs(keyCodes);
		setPossibleMouseInputs(mouseButtons);
	}

	DeviceInputKeyboardMouse::DeviceInputKeyboardMouse(
		const std::vector<int>& keyCodes,
		const std::vector<int>& mouseButtons
	) : MouseScreenPos(0.0f, 0.0f),
		MouseScrollOffset(0.0f, 0.0f)
	{
		setPossibleKeyInputs(keyCodes);
		setPossibleMouseInputs(mouseButtons);
	}

	void DeviceInputKeyboardMouse::setPossibleKeyInputs(const std::vector<int>& keyCodes) {
		PressedKeysMap.clear();

		const size_t keyCodesCount = keyCodes.size();
		if (keyCodesCount > 0) {
			PressedKeysMap.reserve(keyCodesCount);
			for (int keyCode : keyCodes) {
				PressedKeysMap.emplace(keyCode, EPressedState::NOT_PRESSED);
			}
		}
	}

	void DeviceInputKeyboardMouse::setPossibleMouseInputs(const std::vector<int>& mouseButtons) {
		PressedMouseButtonsMap.clear();

		const size_t mouseButtonsCount = mouseButtons.size();
		if (mouseButtonsCount > 0) {
			PressedMouseButtonsMap.reserve(mouseButtonsCount);
			for (int mouseButton : mouseButtons) {
				PressedMouseButtonsMap.emplace(mouseButton, EPressedState::NOT_PRESSED);
			}
		}
	}

	bool DeviceInputKeyboardMouse::setKeyPressed(int keyCode, bool pressed) {
		auto itr = PressedKeysMap.find(keyCode);
		if (itr != PressedKeysMap.end() && itr->second != EPressedState::DISABLED) {
			itr->second = pressed ? EPressedState::PRESSED : EPressedState::NOT_PRESSED;
			return true;
		} else {
			return false;
		}
	}

	bool DeviceInputKeyboardMouse::setMouseButtonPressed(int button, bool pressed) {
		const auto& itr = PressedMouseButtonsMap.find(button);
		if (itr != PressedMouseButtonsMap.end() && itr->second != EPressedState::DISABLED) {
			itr->second = pressed ? EPressedState::PRESSED : EPressedState::NOT_PRESSED;
			return true;
		} else {
			return false;
		}
	}

	bool DeviceInputKeyboardMouse::isKeyPressed(int keyCode) const {
		const auto& itr = PressedKeysMap.find(keyCode);
		if (itr != PressedKeysMap.end()) {
			return itr->second == EPressedState::PRESSED;
		} else {
			LOG_WARN("Key not in current key map: " << keyCode);
			return false;
		}
	}

	bool DeviceInputKeyboardMouse::isMouseButtonPressed(int button) const {
		const auto& itr = PressedMouseButtonsMap.find(button);
		if (itr != PressedMouseButtonsMap.end()) {
			return itr->second == EPressedState::PRESSED;
		} else {
			LOG_WARN("Mouse button not in current mouse button map: " << button);
			return false;
		}
	}

	bool DeviceInputKeyboardMouse::isKeyPressedConsume(int keyCode) {
		std::unordered_map<int, EPressedState>::iterator iter = PressedKeysMap.find(keyCode);
		if (iter != PressedKeysMap.end()) {
			if (iter->second == EPressedState::PRESSED) {
				iter->second = EPressedState::NOT_PRESSED;
				return true;
			}
		}

		return false;
	}

	bool DeviceInputKeyboardMouse::isMouseButtonPressedConsume(int button) {
		std::unordered_map<int, EPressedState>::iterator iter = PressedMouseButtonsMap.find(button);
		if (iter != PressedMouseButtonsMap.end()) {
			if (iter->second == EPressedState::PRESSED) {
				iter->second = EPressedState::NOT_PRESSED;
				return true;
			}
		}

		return false;
	}

	void DeviceInputKeyboardMouse::reset() {
		for (auto& key : PressedKeysMap) {
			key.second = EPressedState::NOT_PRESSED;
		}

		for (auto& button : PressedMouseButtonsMap) {
			button.second = EPressedState::NOT_PRESSED;
		}

		resetCycleData();
	}

	void DeviceInputKeyboardMouse::setKeyCode(int keyCode, EPressedState state) {
		const auto& itr = PressedKeysMap.find(keyCode);
		if (itr != PressedKeysMap.end()) {
			itr->second = state;
		}
	}

	void DeviceInputKeyboardMouse::setMouseButton(int button, EPressedState state) {
		const auto& itr = PressedMouseButtonsMap.find(button);
		if (itr != PressedMouseButtonsMap.end()) {
			itr->second = state;
		}
	}
}
