#pragma once

namespace DOH {

	enum class EEventType {
		NONE = 0,

		//Application
 
		
		//Window
		WINDOW_RESIZE,
		WINDOW_CLOSE,
		WINDOW_FOCUS_CHANGE,
		WINDOW_ICONIFY_CHANGE,
		WINDOW_MONITOR_CHANGE,
		WINDOW_DISPLAY_MODE_CHANGE,

		//Keyboard
		KEY_DOWN,
		KEY_UP,

		//Mouse
		MOUSE_BUTTON_DOWN,
		MOUSE_BUTTON_UP,
		MOUSE_SCROLL,
		MOUSE_MOVE

	};
}
