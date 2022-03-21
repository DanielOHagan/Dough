#pragma once

#include "dough/rendering/Config.h"

namespace DOH {

	class Input {

		friend class Application;

	private:
		static std::unique_ptr<Input> INSTANCE;

		static const std::array<int, 44> DEFAULT_KEY_CODES;
		static const std::array<int, 3> DEFAULT_MOUSE_BUTTON_CODES;

		std::map<int, bool> mPressedKeysMap;
		std::map<int, bool> mPressedMouseButtonsMap;

		std::unique_ptr<glm::vec2> mMouseScreenPos;
		std::unique_ptr<glm::vec2> mMouseScrollOffset;

	private:
		Input(const Input& copy) = delete;
		Input operator=(const Input& assignment) = delete;

		bool isKeyPressedImpl(int keyCode);
		bool isMouseButtonPressedImpl(int button);
		inline bool isMouseScrollingUpImpl() const { return mMouseScrollOffset->y > 0.0f; };
		inline bool isMouseScrollingDownImpl() const { return mMouseScrollOffset->y < 0.0f; };

		inline bool isKeyCodeInPossibleMap(int keyCode) const { return mPressedKeysMap.find(keyCode) != mPressedKeysMap.end(); }
		inline bool isMouseButtonInPossibleMap(int button) const { return mPressedMouseButtonsMap.find(button) != mPressedMouseButtonsMap.end(); }

		void onKeyEvent(int keyCode, bool pressed);
		void onMouseButtonEvent(int button, bool pressed);
		inline void onMouseMove(float x, float y) { mMouseScreenPos->x = x; mMouseScreenPos->y = y; };
		inline void onMouseScroll(float offsetX, float offsetY) { mMouseScrollOffset->x = offsetX; mMouseScrollOffset->y = offsetY; };
		inline void resetCycleData() { mMouseScrollOffset->x = 0.0f; mMouseScrollOffset->y = 0.0f; };

		static void init();
		static void close();
		inline static Input& get() { return *INSTANCE; }

		void setKeyPressedFlag(int keyCode, bool state);
		void setMouseButtonPressedFlag(int button, bool state);

	public:
		Input();

		void setPossibleKeyInputs(std::vector<int>& keyCodes);
		void setPossibleMouseInputs(std::vector<int>& buttons);

		//TODO::
		//void setEnabledPossibleKeyCode(int keyCode, bool enabled);
		//void setEnabledPossibleMouseButton(int button, bool enabled);
		//std::vector<int> getAllPossibleKeyCodes();
		//std::vector<int> getAllPossibleMouseButtons();

		inline static glm::vec2& getCursorPos() { return *INSTANCE->mMouseScreenPos; }
		inline static glm::vec2& getScrollOffset() { return *INSTANCE->mMouseScrollOffset; }

		inline static bool isKeyPressed(int keyCode) { return INSTANCE->isKeyPressedImpl(keyCode); };
		inline static bool isMouseButtonPressed(int button) { return INSTANCE->isMouseButtonPressedImpl(button); };
		inline static bool isMouseScrollingUp() { return INSTANCE->isMouseScrollingUpImpl(); }
		inline static bool isMouseScrollingDown() { return INSTANCE->isMouseScrollingDownImpl(); }

		//TODO::
		//static std::string getKeyName(int keyCode);
		//static std::string getMouseButtonName(int button);
	};
}
