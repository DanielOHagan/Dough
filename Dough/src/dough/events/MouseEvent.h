#pragma once

#include "dough/events/AEvent.h"

namespace DOH {

	class MouseEvent : public AEvent {

	private:


	public:
		MouseEvent(EEventType type)
		:	AEvent(type, EEventCategory::MOUSE)
		{}
	};

	class MouseButtonEvent : public MouseEvent {

	protected:
		int mButton;

		MouseButtonEvent(EEventType type, int button)
		:	MouseEvent(type),
			mButton(button)
		{}

	public:
		inline int getButton() const { return mButton; }
	};

	class MouseButtonDownEvent : public MouseButtonEvent {

	public:
		MouseButtonDownEvent(int button)
		:	MouseButtonEvent(EEventType::MOUSE_BUTTON_DOWN, button)
		{}
	};

	class MouseButtonUpEvent : public MouseButtonEvent {
	
	public:
		MouseButtonUpEvent(int button)
		:	MouseButtonEvent(EEventType::MOUSE_BUTTON_UP, button)
		{}
	};

	class MouseScrollEvent : public MouseEvent {

	private:
		float mOffsetX;
		float mOffsetY;

	public:
		MouseScrollEvent(float offsetX, float offsetY)
		:	MouseEvent(EEventType::MOUSE_SCROLL),
			mOffsetX(offsetX),
			mOffsetY(offsetY)
		{}

	};

	class MouseMoveEvent : public MouseEvent {
	
	private:
		float mPosX;
		float mPosY;

	public:
		MouseMoveEvent(float posX, float posY)
		:	MouseEvent(EEventType::MOUSE_MOVE),
			mPosX(posX),
			mPosY(posY)
		{}

		inline float getPosX() const { return mPosX; }
		inline float getPosY() const { return mPosY; }
	};
}
