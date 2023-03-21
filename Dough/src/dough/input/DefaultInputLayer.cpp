#include "dough/input/DefaultInputLayer.h"

namespace DOH {

	DefaultInputLayer::DefaultInputLayer()
		: AInputLayer(),
		mKeyboardMouseInput(std::make_shared<DeviceInputKeyboardMouse>()) {}

	bool DefaultInputLayer::handleKeyPressed(int keyCode, bool pressed) {
		if (mKeyboardMouseInput != nullptr) {
			return mKeyboardMouseInput->setKeyPressed(keyCode, pressed);
		} else {
			return false;
		}
	}

	bool DefaultInputLayer::handleMouseButtonPressed(int button, bool pressed) {
		if (mKeyboardMouseInput != nullptr) {
			return mKeyboardMouseInput->setMouseButtonPressed(button, pressed);
		} else {
			return false;
		}
	}

	bool DefaultInputLayer::handleMouseMoved(float x, float y) {
		if (mKeyboardMouseInput != nullptr) {
			mKeyboardMouseInput->setMouseScreenPos(x, y);
			return true;
		} else {
			return false;
		}
	}

	bool DefaultInputLayer::handleMouseScroll(float offsetX, float offsetY) {
		if (mKeyboardMouseInput != nullptr) {
			mKeyboardMouseInput->setMouseScrollOffset(offsetX, offsetY);
			return true;
		} else {
			return false;
		}
	}
}
