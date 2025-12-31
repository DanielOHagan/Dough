#pragma once

#include "dough/Core.h"
#include "dough/input/AInputLayer.h"
#include "dough/input/DeviceInput.h"
#include "dough/input/InputActionMap.h"

namespace DOH {

	class DefaultInputLayer : public AInputLayer {
	private:
		std::shared_ptr<DeviceInputKeyboardMouse> mKeyboardMouseInput;
		//TODO:: std::shared_ptr<DeviceInputController> mControllerInput; //a.k.a Gamepad
		std::unique_ptr<InputActionMap> mInputActionMapping;

	public:
		DefaultInputLayer(const char* name);
		DefaultInputLayer(const char* name, std::shared_ptr<DeviceInputKeyboardMouse> keyboardMouseInput);

		virtual bool handleKeyPressed(int keyCode, bool pressed) override;
		virtual bool handleMouseButtonPressed(int button, bool pressed) override;
		virtual bool handleMouseMoved(float x, float y) override;
		virtual bool handleMouseScroll(float offsetX, float offsetY) override;
		virtual void resetCycleData() override { mKeyboardMouseInput->resetCycleData(); }
		virtual void reset() override { mKeyboardMouseInput->reset(); };

		virtual inline bool hasDeviceInput() const override { return mKeyboardMouseInput != nullptr; }

		virtual inline bool isKeyPressed(int keyCode) const override { return mKeyboardMouseInput->isKeyPressed(keyCode); }
		virtual inline bool isMouseButtonPressed(int button) const override { return mKeyboardMouseInput->isMouseButtonPressed(button); }
		virtual inline bool isMouseScrollingUp() const override { return mKeyboardMouseInput->isMouseScrollingUp(); }
		virtual inline bool isMouseScrollingDown() const override { return mKeyboardMouseInput->isMouseScrollingDown(); }
		virtual inline bool isActionActiveAND(const char* action) const override { return mInputActionMapping->isActionActiveAND(action, *this); }
		virtual inline bool isActionActiveOR(const char* action) const override { return mInputActionMapping->isActionActiveOR(action, *this); }
		virtual inline const glm::vec2 getCursorPos() const override { return mKeyboardMouseInput->getCursorPos(); }
		virtual inline bool isKeyPressedConsume(int keyCode) override { return mKeyboardMouseInput->isKeyPressedConsume(keyCode); }
		virtual inline bool isMouseButtonPressedConsume(int button) override { return mKeyboardMouseInput->isMouseButtonPressedConsume(button); }
		virtual inline bool isMouseScrollingUpConsume() override { return mKeyboardMouseInput->isMouseScrollingUpConsume(); }
		virtual inline bool isMouseScrollingDownConsume() override { return mKeyboardMouseInput->isMouseScrollingDownConsume(); }
		virtual inline bool isActionActiveANDConsume(const char* action) override { return mInputActionMapping->isActionActiveANDConsume(action, *this); }
		virtual inline bool isActionActiveORConsume(const char* action) override { return mInputActionMapping->isActionActiveORConsume(action, *this); }

		virtual void consumeAction(InputAction& action) override;

		inline InputActionMap& getActionMap() { return *mInputActionMapping; }
	};
}
