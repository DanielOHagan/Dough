#pragma once

#include "dough/Core.h"
#include "dough/input/AInputLayer.h"
#include "dough/input/DeviceInput.h"

namespace DOH {

	class DefaultInputLayer : public AInputLayer {
	private:
		std::shared_ptr<DeviceInputKeyboardMouse> mKeyboardMouseInput;

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

		virtual bool isKeyPressed(int keyCode) const override { return mKeyboardMouseInput->isKeyPressed(keyCode); }
		virtual bool isMouseButtonPressed(int button) const override { return mKeyboardMouseInput->isMouseButtonPressed(button); }
		virtual inline bool isMouseScrollingUp() const override { return mKeyboardMouseInput->isMouseScrollingUp(); }
		virtual inline bool isMouseScrollingDown() const override { return mKeyboardMouseInput->isMouseScrollingDown(); }
		virtual inline const glm::vec2 getCursorPos() const override { return mKeyboardMouseInput->getCursorPos(); }
	};
}
