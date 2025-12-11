#include "dough/input/DefaultInputLayer.h"

namespace DOH {

	DefaultInputLayer::DefaultInputLayer(const char* name)
	:	AInputLayer(name),
		mKeyboardMouseInput(std::make_shared<DeviceInputKeyboardMouse>()),
		mInputActionMapping(std::make_unique<InputActionMap>())
	{}

	DefaultInputLayer::DefaultInputLayer(const char* name, std::shared_ptr<DeviceInputKeyboardMouse> keyboardMouseInput)
	:	AInputLayer(name),
		mKeyboardMouseInput(keyboardMouseInput),
		mInputActionMapping(std::make_unique<InputActionMap>())
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

	void DefaultInputLayer::consumeAction(InputAction& action) {
		for (std::pair<EDeviceInputType, int> actionStep : action.ActionCodesByDevice) {
			switch (actionStep.first) {
				case EDeviceInputType::KEY_PRESS:
					mKeyboardMouseInput->setKeyPressed(actionStep.second, false);;
					break;
				case EDeviceInputType::MOUSE_PRESS:
					mKeyboardMouseInput->setMouseButtonPressed(actionStep.second, false);
					break;
				case EDeviceInputType::MOUSE_SCROLL_DOWN:
					mKeyboardMouseInput->setMouseScrollOffset(mKeyboardMouseInput->getScrollOffset().x, 0.0f);
					break;
				case EDeviceInputType::MOUSE_SCROLL_UP:
					mKeyboardMouseInput->setMouseScrollOffset(mKeyboardMouseInput->getScrollOffset().x, 0.0f);
					break;
				case EDeviceInputType::MOUSE_MOVE:
					continue; //Mouse movement can by handled by "Input::getMousePos()"

					//NONE is used to show that no more actions are expected.
				default:
				case EDeviceInputType::NONE:
					return;
			}
		}
	}
}
