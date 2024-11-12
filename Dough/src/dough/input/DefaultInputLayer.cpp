#include "dough/input/DefaultInputLayer.h"

namespace DOH {

	DefaultInputLayer::DefaultInputLayer(const char* name)
	:	AInputLayer(name),
		mKeyboardMouseInput(std::make_shared<DeviceInputKeyboardMouse>())
	{}

	DefaultInputLayer::DefaultInputLayer(const char* name, std::shared_ptr<DeviceInputKeyboardMouse> keyboardMouseInput)
	:	AInputLayer(name),
		mKeyboardMouseInput(keyboardMouseInput)
	{}

	bool DefaultInputLayer::handleKeyPressed(int keyCode, bool pressed) {
		return mKeyboardMouseInput->setKeyPressed(keyCode, pressed);
	}

	bool DefaultInputLayer::handleMouseButtonPressed(int button, bool pressed) {
		return mKeyboardMouseInput->setMouseButtonPressed(button, pressed);
	}

	bool DefaultInputLayer::handleMouseMoved(float x, float y) {
		mKeyboardMouseInput->setMouseScreenPos(x, y);
		return true;
	}

	bool DefaultInputLayer::handleMouseScroll(float offsetX, float offsetY) {
		mKeyboardMouseInput->setMouseScrollOffset(offsetX, offsetY);
		return true;
	}
}
