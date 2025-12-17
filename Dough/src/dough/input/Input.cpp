#include "dough/input/Input.h"

#include "dough/input/InputCodes.h"
#include "dough/Logging.h"

namespace DOH {

	std::unique_ptr<Input> Input::INSTANCE = nullptr;

	const std::array<int, 120> Input::ALL_KEY_CODES = {
		DOH_KEY_SPACE, DOH_KEY_APOSTROPHE, DOH_KEY_COMMA, DOH_KEY_MINUS,
		DOH_KEY_FULL_STOP, DOH_KEY_FORWARD_SLASH,
		DOH_KEY_0, DOH_KEY_1, DOH_KEY_2, DOH_KEY_3, DOH_KEY_4, DOH_KEY_5,
		DOH_KEY_6, DOH_KEY_7, DOH_KEY_8, DOH_KEY_9,
		DOH_KEY_SEMICOLON, DOH_KEY_EQUAL,
		DOH_KEY_A, DOH_KEY_B, DOH_KEY_C, DOH_KEY_D, DOH_KEY_E, DOH_KEY_F,
		DOH_KEY_G, DOH_KEY_H, DOH_KEY_I, DOH_KEY_J, DOH_KEY_K, DOH_KEY_L,
		DOH_KEY_M, DOH_KEY_N, DOH_KEY_O, DOH_KEY_P, DOH_KEY_Q, DOH_KEY_R,
		DOH_KEY_S, DOH_KEY_T, DOH_KEY_U, DOH_KEY_V, DOH_KEY_W, DOH_KEY_X,
		DOH_KEY_Y, DOH_KEY_Z,
		DOH_KEY_LEFT_BRACKET, DOH_KEY_BACK_SLASH, DOH_KEY_RIGHT_BRACKET,
		DOH_KEY_GRAVE_ACCENT, DOH_KEY_WORLD_1, DOH_KEY_WORLD_2,
		DOH_KEY_ESCAPE, DOH_KEY_ENTER, DOH_KEY_TAB, DOH_KEY_BACKSPACE,
		DOH_KEY_INSERT, DOH_KEY_DELETE,
		DOH_KEY_RIGHT, DOH_KEY_LEFT, DOH_KEY_DOWN, DOH_KEY_UP,
		DOH_KEY_PAGE_UP, DOH_KEY_PAGE_DOWN, DOH_KEY_HOME, DOH_KEY_END,
		DOH_KEY_CAPS_LOCK, DOH_KEY_SCROLL_LOCK, DOH_KEY_NUM_LOCK,
		DOH_KEY_PRINT_SCREEN, DOH_KEY_PAUSE,
		DOH_KEY_F1, DOH_KEY_F2, DOH_KEY_F3, DOH_KEY_F4, DOH_KEY_F5, DOH_KEY_F6,
		DOH_KEY_F7, DOH_KEY_F8, DOH_KEY_F9, DOH_KEY_F10, DOH_KEY_F11, DOH_KEY_F12,
		DOH_KEY_F13, DOH_KEY_F14, DOH_KEY_F15, DOH_KEY_F16, DOH_KEY_F17, DOH_KEY_F18,
		DOH_KEY_F19, DOH_KEY_F20, DOH_KEY_F21, DOH_KEY_F22, DOH_KEY_F23, DOH_KEY_F24,
		DOH_KEY_F25,
		DOH_KEY_KP_0, DOH_KEY_KP_1, DOH_KEY_KP_2, DOH_KEY_KP_3, DOH_KEY_KP_4,
		DOH_KEY_KP_5, DOH_KEY_KP_6, DOH_KEY_KP_7, DOH_KEY_KP_8, DOH_KEY_KP_9,
		DOH_KEY_KP_DECIMAL, DOH_KEY_KP_DIVIDE, DOH_KEY_KP_MULTIPLY, DOH_KEY_KP_SUBTRACT,
		DOH_KEY_KP_ADD, DOH_KEY_KP_ENTER, DOH_KEY_KP_EQUAL,
		DOH_KEY_LEFT_SHIFT, DOH_KEY_LEFT_CONTROL, DOH_KEY_LEFT_ALT, DOH_KEY_LEFT_SUPER,
		DOH_KEY_RIGHT_SHIFT, DOH_KEY_RIGHT_CONTROL, DOH_KEY_RIGHT_ALT, DOH_KEY_RIGHT_SUPER,
		DOH_KEY_MENU
	};

	const std::array<int, 53> Input::DEFAULT_KEY_CODES = {
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
		DOH_KEY_ESCAPE, DOH_KEY_SPACE, DOH_KEY_ENTER, DOH_KEY_LEFT_SHIFT,
		DOH_KEY_RIGHT, DOH_KEY_LEFT, DOH_KEY_UP, DOH_KEY_DOWN,
		DOH_KEY_F1, DOH_KEY_F2, DOH_KEY_F3, DOH_KEY_F4, DOH_KEY_F5
	};

	const std::array<int, 8> Input::DEFAULT_MOUSE_BUTTON_CODES = {
		DOH_MOUSE_BUTTON_1, DOH_MOUSE_BUTTON_2, DOH_MOUSE_BUTTON_3, DOH_MOUSE_BUTTON_4,
		DOH_MOUSE_BUTTON_5, DOH_MOUSE_BUTTON_6, DOH_MOUSE_BUTTON_7, DOH_MOUSE_BUTTON_8
	};

	Input::Input()
	:	mMousePosX(0.0f),
		mMousePosY(0.0f)
	{}

	void Input::init() {
		INSTANCE = std::make_unique<Input>();
	}

	void Input::close() {
		if (INSTANCE != nullptr) {
			INSTANCE->mInputLayers.clear();
			INSTANCE.reset();
			INSTANCE = nullptr;
		}
	}

	void Input::onKeyPressedEvent(int keyCode, bool pressed) {
		for (auto& inputLayer : mInputLayers) {
			if (inputLayer->isEnabled() && inputLayer->handleKeyPressed(keyCode, pressed)) {
				return;
			}
		}
	}

	void Input::onMouseButtonPressedEvent(int button, bool pressed) {
		for (auto& inputLayer : mInputLayers) {
			if (inputLayer->isEnabled() && inputLayer->handleMouseButtonPressed(button, pressed)) {
				return;
			}
		}
	}

	void Input::onMouseMoveEvent(float x, float y) {
		mMousePosX = x;
		mMousePosY = y;
		for (auto& inputLayer : mInputLayers) {
			if (inputLayer->isEnabled() && inputLayer->handleMouseMoved(x, y)) {
				return;
			}
		}
	}

	void Input::onMouseScrollEvent(float offsetX, float offsetY) {
		for (auto& inputLayer : mInputLayers) {
			if (inputLayer->isEnabled() && inputLayer->handleMouseScroll(offsetX, offsetY)) {
				return;
			}
		}
	}

	void Input::resetCycleData() {
		for (auto& inputLayer : mInputLayers) {
			inputLayer->resetCycleData();
		}
	}

	void Input::addInputLayer(std::shared_ptr<AInputLayer> inputLayer) {
		for (auto itr = INSTANCE->mInputLayers.begin(); itr != INSTANCE->mInputLayers.end(); itr++) {
			if (strcmp(itr->get()->getName(), inputLayer->getName()) == 0) {
				LOG_ERR("Input Layer add failed, name taken: " << inputLayer->getName());
				return;
			}
		}

		INSTANCE->mInputLayers.emplace_back(inputLayer);
	}

	void Input::removeInputLayer(const char* name) {
		for (auto itr = INSTANCE->mInputLayers.begin(); itr != INSTANCE->mInputLayers.end(); itr++) {
			if (strcmp(itr->get()->getName(), name) == 0) {
				INSTANCE->mInputLayers.erase(itr);
				return;
			}
		}

		LOG_WARN("Input Layer not found when trying to remove: " << name);
	}

	std::vector<std::shared_ptr<AInputLayer>>& Input::getInputLayers() {
		return INSTANCE->mInputLayers;
	}

	std::optional<std::reference_wrapper<AInputLayer>> Input::getInputLayer(const char* name) {
		for (auto itr = INSTANCE->mInputLayers.begin(); itr != INSTANCE->mInputLayers.end(); itr++) {
			if (strcmp(itr->get()->getName(), name) == 0) {
				return { *itr->get() };
			}
		}

		LOG_ERR("Failed to get Input Layer: " << name);
		return {};
	}

	std::optional<std::shared_ptr<AInputLayer>> Input::getInputLayerPtr(const char* name) {
		for (auto itr = INSTANCE->mInputLayers.begin(); itr != INSTANCE->mInputLayers.end(); itr++) {
			if (strcmp(itr->get()->getName(), name) == 0) {
				auto d = itr->get();
				return { *itr };
			}
		}

		LOG_ERR("Failed to get Input Layer: " << name);
		return {};
	}

	const char* Input::getInputString(int keyCode) {
		switch (keyCode) {
			case 0: return "MOUSE_BUTTON_1-MOUSE_BUTTON_LEFT";
			case 1: return "MOUSE_BUTTON_2-MOUSE_BUTTON_RIGHT";
			case 2: return "MOUSE_BUTTON_3-MOUSE_BUTTON_MIDDLE";
			case 3: return "MOUSE_BUTTON_4";
			case 4: return "MOUSE_BUTTON_5";
			case 5: return "MOUSE_BUTTON_6";
			case 6: return "MOUSE_BUTTON_7";
			case 7: return "MOUSE_BUTTON_8-DOH_MOUSE_BUTTON_LAST";
			case 32: return "KEY_SPACE";
			case 39: return "KEY_APOSTROPHE";
			case 44: return "KEY_COMMA";
			case 45: return "KEY_MINUS";
			case 46: return "KEY_FULL_STOP";
			case 47: return "KEY_FORWARD_SLASH";
			case 48: return "KEY_0";
			case 49: return "KEY_1";
			case 50: return "KEY_2";
			case 51: return "KEY_3";
			case 52: return "KEY_4";
			case 53: return "KEY_5";
			case 54: return "KEY_6";
			case 55: return "KEY_7";
			case 56: return "KEY_8";
			case 57: return "KEY_9";
			case 59: return "KEY_SEMICOLON";
			case 61: return "KEY_EQUAL";
			case 65: return "KEY_A";
			case 66: return "KEY_B";
			case 67: return "KEY_C";
			case 68: return "KEY_D";
			case 69: return "KEY_E";
			case 70: return "KEY_F";
			case 71: return "KEY_G";
			case 72: return "KEY_H";
			case 73: return "KEY_I";
			case 74: return "KEY_J";
			case 75: return "KEY_K";
			case 76: return "KEY_L";
			case 77: return "KEY_M";
			case 78: return "KEY_N";
			case 79: return "KEY_O";
			case 80: return "KEY_P";
			case 81: return "KEY_Q";
			case 82: return "KEY_R";
			case 83: return "KEY_S";
			case 84: return "KEY_T";
			case 85: return "KEY_U";
			case 86: return "KEY_V";
			case 87: return "KEY_W";
			case 88: return "KEY_X";
			case 89: return "KEY_Y";
			case 90: return "KEY_Z";
			case 91: return "KEY_LEFT_BRACKET";
			case 92: return "KEY_BACK_SLASH";
			case 93: return "KEY_RIGHT_BRACKET";
			case 96: return "KEY_GRAVE_ACCENT";
			case 161: return "KEY_WORLD_1";
			case 162: return "KEY_WORLD_2";
			case 256: return "KEY_ESCAPE";
			case 257: return "KEY_ENTER";
			case 258: return "KEY_TAB";
			case 259: return "KEY_BACKSPACE";
			case 260: return "KEY_INSERT";
			case 261: return "KEY_DELETE";
			case 262: return "KEY_RIGHT";
			case 263: return "KEY_LEFT";
			case 264: return "KEY_DOWN";
			case 265: return "KEY_UP";
			case 266: return "KEY_PAGE_UP";
			case 267: return "KEY_PAGE_DOWN";
			case 268: return "KEY_HOME";
			case 269: return "KEY_END";
			case 280: return "KEY_CAPS_LOCK";
			case 281: return "KEY_SCROLL_LOCK";
			case 282: return "KEY_NUM_LOCK";
			case 283: return "KEY_PRINT_SCREEN";
			case 284: return "KEY_PAUSE";
			case 290: return "KEY_F1";
			case 291: return "KEY_F2";
			case 292: return "KEY_F3";
			case 293: return "KEY_F4";
			case 294: return "KEY_F5";
			case 295: return "KEY_F6";
			case 296: return "KEY_F7";
			case 297: return "KEY_F8";
			case 298: return "KEY_F9";
			case 299: return "KEY_F10";
			case 300: return "KEY_F11";
			case 301: return "KEY_F12";
			case 302: return "KEY_F13";
			case 303: return "KEY_F14";
			case 304: return "KEY_F15";
			case 305: return "KEY_F16";
			case 306: return "KEY_F17";
			case 307: return "KEY_F18";
			case 308: return "KEY_F19";
			case 309: return "KEY_F20";
			case 310: return "KEY_F21";
			case 311: return "KEY_F22";
			case 312: return "KEY_F23";
			case 313: return "KEY_F24";
			case 314: return "KEY_F25";
			case 320: return "KEY_KP_0";
			case 321: return "KEY_KP_1";
			case 322: return "KEY_KP_2";
			case 323: return "KEY_KP_3";
			case 324: return "KEY_KP_4";
			case 325: return "KEY_KP_5";
			case 326: return "KEY_KP_6";
			case 327: return "KEY_KP_7";
			case 328: return "KEY_KP_8";
			case 329: return "KEY_KP_9";
			case 330: return "KEY_KP_DECIMAL";
			case 331: return "KEY_KP_DIVIDE";
			case 332: return "KEY_KP_MULTIPLY";
			case 333: return "KEY_KP_SUBTRACT";
			case 334: return "KEY_KP_ADD";
			case 335: return "KEY_KP_ENTER";
			case 336: return "KEY_KP_EQUAL";
			case 340: return "KEY_LEFT_SHIFT";
			case 341: return "KEY_LEFT_CONTROL";
			case 342: return "KEY_LEFT_ALT";
			case 343: return "KEY_LEFT_SUPER";
			case 344: return "KEY_RIGHT_SHIFT";
			case 345: return "KEY_RIGHT_CONTROL";
			case 346: return "KEY_RIGHT_ALT";
			case 347: return "KEY_RIGHT_SUPER";
			case 348: return "KEY_MENU";

			default:
				//Empty codes, ranges are inclusive.
				//8 - 31
				//33 - 38
				//41 - 43
				//60
				//62 - 64
				//94 - 95
				//97 - 160
				//163 - 255
				//270 - 279
				//285 - 289
				//315 - 319
				//337 - 339
				return "EMPTY";
		}
	}

	const char* Input::getInputStringShortHand(int keyCode) {
		switch (keyCode) {
			case 0: return "MOUSE_LEFT";
			case 1: return "MOUSE_RIGHT";
			case 2: return "MOUSE_MIDDLE";
			case 3: return "MOUSE_4";
			case 4: return "MOUSE_5";
			case 5: return "MOUSE_6";
			case 6: return "MOUSE_7";
			case 7: return "MOUSE_LAST";
			case 32: return "SPACE";
			case 39: return "'";
			case 44: return ",";
			case 45: return "-";
			case 46: return ".";
			case 47: return "/";
			case 48: return "0";
			case 49: return "1";
			case 50: return "2";
			case 51: return "3";
			case 52: return "4";
			case 53: return "5";
			case 54: return "6";
			case 55: return "7";
			case 56: return "8";
			case 57: return "9";
			case 59: return ";";
			case 61: return "=";
			case 65: return "A";
			case 66: return "B";
			case 67: return "C";
			case 68: return "D";
			case 69: return "E";
			case 70: return "F";
			case 71: return "G";
			case 72: return "H";
			case 73: return "I";
			case 74: return "J";
			case 75: return "K";
			case 76: return "L";
			case 77: return "M";
			case 78: return "N";
			case 79: return "O";
			case 80: return "P";
			case 81: return "Q";
			case 82: return "R";
			case 83: return "S";
			case 84: return "T";
			case 85: return "U";
			case 86: return "V";
			case 87: return "W";
			case 88: return "X";
			case 89: return "Y";
			case 90: return "Z";
			case 91: return "[";
			case 92: return "\\";
			case 93: return "]";
			case 96: return "`";
			case 161: return "WORLD_1";
			case 162: return "WORLD_2";
			case 256: return "ESCAPE";
			case 257: return "ENTER";
			case 258: return "TAB";
			case 259: return "BACKSPACE";
			case 260: return "INSERT";
			case 261: return "DELETE";
			case 262: return "RIGHT";
			case 263: return "LEFT";
			case 264: return "DOWN";
			case 265: return "UP";
			case 266: return "PAGE_UP";
			case 267: return "PAGE_DOWN";
			case 268: return "HOME";
			case 269: return "END";
			case 280: return "CAPS_LOCK";
			case 281: return "SCROLL_LOCK";
			case 282: return "NUM_LOCK";
			case 283: return "PRINT_SCREEN";
			case 284: return "PAUSE";
			case 290: return "F1";
			case 291: return "F2";
			case 292: return "F3";
			case 293: return "F4";
			case 294: return "F5";
			case 295: return "F6";
			case 296: return "F7";
			case 297: return "F8";
			case 298: return "F9";
			case 299: return "F10";
			case 300: return "F11";
			case 301: return "F12";
			case 302: return "F13";
			case 303: return "F14";
			case 304: return "F15";
			case 305: return "F16";
			case 306: return "F17";
			case 307: return "F18";
			case 308: return "F19";
			case 309: return "F20";
			case 310: return "F21";
			case 311: return "F22";
			case 312: return "F23";
			case 313: return "F24";
			case 314: return "F25";
			case 320: return "KP_0";
			case 321: return "KP_1";
			case 322: return "KP_2";
			case 323: return "KP_3";
			case 324: return "KP_4";
			case 325: return "KP_5";
			case 326: return "KP_6";
			case 327: return "KP_7";
			case 328: return "KP_8";
			case 329: return "KP_9";
			case 330: return "KP_DECIMAL";
			case 331: return "KP_DIVIDE";
			case 332: return "KP_MULTIPLY";
			case 333: return "KP_SUBTRACT";
			case 334: return "KP_ADD";
			case 335: return "KP_ENTER";
			case 336: return "KP_EQUAL";
			case 340: return "LEFT_SHIFT";
			case 341: return "LEFT_CONTROL";
			case 342: return "LEFT_ALT";
			case 343: return "LEFT_SUPER";
			case 344: return "RIGHT_SHIFT";
			case 345: return "RIGHT_CONTROL";
			case 346: return "RIGHT_ALT";
			case 347: return "RIGHT_SUPER";
			case 348: return "MENU";

			default:
				//Empty codes, ranges are inclusive.
				//8 - 31
				//33 - 38
				//41 - 43
				//60
				//62 - 64
				//94 - 95
				//97 - 160
				//163 - 255
				//270 - 279
				//285 - 289
				//315 - 319
				//337 - 339
				return "EMPTY";
		}
	}
}
