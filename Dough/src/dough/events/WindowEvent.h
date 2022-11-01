#pragma once

#include "dough/events/AEvent.h"
#include "dough/Window.h"

namespace DOH {

	class WindowEvent : public AEvent {

	private:
		Window& mWindow;

	protected:
		WindowEvent(EEventType type, Window& window)
		:	AEvent(type, EEventCategory::WINDOW),
			mWindow(window) 
		{}

	public:
		inline Window& getWindow() const { return mWindow; }
	};
	
	class WindowResizeEvent : public WindowEvent {

	private:
		uint32_t mWidth;
		uint32_t mHeight;

	public:
		WindowResizeEvent(Window& window, uint32_t width, uint32_t height)
		:	WindowEvent(EEventType::WINDOW_RESIZE, window),
			mWidth(width),
			mHeight(height)
		{}

		inline uint32_t getWidth() const { return mWidth; }
		inline uint32_t getHeight() const { return mHeight; }
	};

	class WindowCloseEvent : public WindowEvent {

	public:
		WindowCloseEvent(Window& window)
		:	WindowEvent(EEventType::WINDOW_CLOSE, window)
		{}
	};
	
	class WindowFocusChangeEvent : public WindowEvent {
		
	private:
		bool mFocused;

	public:
		WindowFocusChangeEvent(Window& window, bool focused)
		:	WindowEvent(EEventType::WINDOW_FOCUS_CHANGE, window),
			mFocused(focused)
		{}

		inline bool isFocused() const { return mFocused; }
	};

	class WindowIconifyChangeEvent : public WindowEvent {

	private:
		bool mIconified;

	public:
		WindowIconifyChangeEvent(Window& window, bool iconified)
		:	WindowEvent(EEventType::WINDOW_ICONIFY_CHANGE, window),
			mIconified(iconified)
		{}

		inline bool isIconified() const { return mIconified; }
	};

	class WindowMonitorChangeEvent : public WindowEvent {

	private:
		uint32_t mWidth;
		uint32_t mHeight;
		uint32_t mMaxRefreshRate;
		EWindowDisplayMode mDisplayMode;

	public:
		WindowMonitorChangeEvent(Window& window, uint32_t width, uint32_t height, uint32_t maxRefreshRate, EWindowDisplayMode displayMode)
		:	WindowEvent(EEventType::WINDOW_MONITOR_CHANGE, window),
			mWidth(width),
			mHeight(height),
			mMaxRefreshRate(maxRefreshRate),
			mDisplayMode(displayMode)
		{}

		inline uint32_t getWidth() const { return mWidth; }
		inline uint32_t getHeight() const { return mHeight; }
		inline uint32_t getMaxRefreshRate() const { return mMaxRefreshRate; }
		inline EWindowDisplayMode getDisplayMode() const { return mDisplayMode; }
	};

	class WindowDisplayModeChangeEvent : public WindowEvent {
	private:
		EWindowDisplayMode mDisplayMode;

	public:
		WindowDisplayModeChangeEvent(Window& window, EWindowDisplayMode displayMode)
		:	WindowEvent(EEventType::WINDOW_DISPLAY_MODE_CHANGE, window),
			mDisplayMode(displayMode)
		{}

		inline EWindowDisplayMode getDisplayMode() const { return mDisplayMode; }
	};
}
