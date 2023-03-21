#pragma once

#include "dough/Core.h"
#include "dough/Maths.h"

namespace DOH {

	//Keyboard & Mouse input
	struct DeviceInputKeyboardMouse {
		std::unordered_map<int, bool> PressedKeysMap;
		std::unordered_map<int, bool> PressedMouseButtonsMap;

		glm::vec2 MouseScreenPos;
		glm::vec2 MouseScrollOffset;

		DeviceInputKeyboardMouse();
		DeviceInputKeyboardMouse(const std::vector<int>& keyCodes, const std::vector<int>& mouseButtons);

		bool setKeyPressed(int keyCode, bool pressed);
		bool setMouseButtonPressed(int button, bool pressed);
		inline void setMouseScreenPos(float x, float y) {
			MouseScreenPos.x = x;
			MouseScreenPos.y = y;
		}
		inline void setMouseScrollOffset(float offsetX, float offsetY) {
			MouseScrollOffset.x = offsetX;
			MouseScrollOffset.y = offsetY;
		}

		inline void resetCycleData() { MouseScrollOffset.x = 0.0f; MouseScrollOffset.y = 0.0f; };

		bool isKeyPressed(int keyCode) const;
		bool isMouseButtonPressed(int button) const;
		inline bool isMouseScrollingUp() const { return MouseScrollOffset.y > 0.0f; };
		inline bool isMouseScrollingDown() const { return MouseScrollOffset.y < 0.0f; };

		inline bool isKeyCodeInPossibleMap(int keyCode) const {
			return PressedKeysMap.find(keyCode) != PressedKeysMap.end();
		}
		inline bool isMouseButtonInPossibleMap(int button) const {
			return PressedMouseButtonsMap.find(button) != PressedMouseButtonsMap.end();
		}

		void setPossibleKeyInputs(const std::vector<int>& keyCodes);
		inline void setPossibleKeyInputs(const std::unordered_map<int, bool>& keysMap) { PressedKeysMap = keysMap; }
		void setPossibleMouseInputs(const std::vector<int>& mouseButtons);
		inline void setPossibleMouseInputs(const std::unordered_map<int, bool>& mouseButtonsMap) { PressedMouseButtonsMap = mouseButtonsMap; }

		inline const glm::vec2& getCursorPos() const { return MouseScreenPos; }
		inline const glm::vec2& getScrollOffset() const { return MouseScrollOffset; }

		void reset();
	};
}
