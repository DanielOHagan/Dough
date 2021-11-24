#include "dough/input/Input.h"
#include "dough/input/InputCodes.h"

namespace DOH {

	std::unique_ptr<Input> Input::INSTANCE = nullptr;

	const std::array<int, 40> Input::DEFAULT_KEY_CODES = {
		//Alphabet
		DOH_KEY_A, DOH_KEY_B, DOH_KEY_C, DOH_KEY_D, DOH_KEY_E, DOH_KEY_F,
		DOH_KEY_G, DOH_KEY_H, DOH_KEY_I, DOH_KEY_J, DOH_KEY_K, DOH_KEY_L,
		DOH_KEY_M, DOH_KEY_N, DOH_KEY_O, DOH_KEY_P, DOH_KEY_Q, DOH_KEY_R,
		DOH_KEY_S, DOH_KEY_T, DOH_KEY_U, DOH_KEY_V, DOH_KEY_W, DOH_KEY_X,
		DOH_KEY_Y, DOH_KEY_Z,

		//Numeric
		DOH_KEY_0, DOH_KEY_1, DOH_KEY_2, DOH_KEY_3, DOH_KEY_4, DOH_KEY_5,
		DOH_KEY_6, DOH_KEY_7, DOH_KEY_8, DOH_KEY_9,

		//Functions
		DOH_KEY_ESCAPE, DOH_KEY_SPACE, DOH_KEY_ENTER, DOH_KEY_LEFT_SHIFT
	};

	const std::array<int, 3> Input::DEFAULT_MOUSE_BUTTON_CODES = {
		DOH_MOUSE_BUTTON_LEFT, DOH_MOUSE_BUTTON_MIDDLE, DOH_MOUSE_BUTTON_RIGHT
	};

	Input::Input()
	:	mMouseScreenPos(std::make_unique<glm::vec2>(0.0f, 0.0f))
	{}

	void Input::init() {
		//TODO:: pass in possible key and mouse inputs or use some other kind of input masking system
		INSTANCE = std::make_unique<Input>();

		std::vector<int> keyCodes{ DEFAULT_KEY_CODES.begin(),DEFAULT_KEY_CODES.end() };
		std::vector<int> mouseButtons{ DEFAULT_MOUSE_BUTTON_CODES.begin(), DEFAULT_MOUSE_BUTTON_CODES.end() };
		INSTANCE->setPossibleKeyInputs(keyCodes);
		INSTANCE->setPossibleMouseInputs(mouseButtons);
	}

	void Input::close() {
		if (INSTANCE != nullptr) {
			INSTANCE.reset();
			INSTANCE = nullptr;
		}
	}

	void Input::setPossibleKeyInputs(std::vector<int>& keyCodes) {
		INSTANCE->mPressedKeysMap.clear();

		for (int keyCode : keyCodes) {
			INSTANCE->mPressedKeysMap.emplace(keyCode, false);
		}
	}

	void Input::setPossibleMouseInputs(std::vector<int>& buttons) {
		INSTANCE->mPressedMouseButtonsMap.clear();

		for (int btn : buttons) {
			INSTANCE->mPressedMouseButtonsMap.emplace(btn, false);
		}
	}

	void Input::onKeyEvent(int keyCode, bool pressed) {
		if (isKeyCodeInPossibleMap(keyCode)) {
			setKeyPressedFlag(keyCode, pressed);
		}
	}

	void Input::onMouseButtonEvent(int button, bool pressed) {
		if (isMouseButtonInPossibleMap(button)) {
			setMouseButtonPressedFlag(button, pressed);
		}
	}

	void Input::setKeyPressedFlag(int keyCode, bool state) {
		mPressedKeysMap[keyCode] = state;
	}

	void Input::setMouseButtonPressedFlag(int button, bool state) {
		mPressedMouseButtonsMap[button] = state;
	}

	bool Input::isKeyPressedImpl(int keyCode) {
		if (isKeyCodeInPossibleMap(keyCode)) {
			return mPressedKeysMap.at(keyCode);
		}

		//TODO::
		//LogWarning "isKeyPressedImpl asked of key not in possible key codes"

		return false;
	}

	bool Input::isMouseButtonPressedImpl(int button) {
		if (isMouseButtonInPossibleMap(button)) {
			return mPressedMouseButtonsMap.at(button);
		}

		//TODO::
		//LogWarning "isMouseButtonPressedImpl asked of button not in possible mouse button codes"

		return false;
	}
}
