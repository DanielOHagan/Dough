#pragma once

#include "dough/events/AEvent.h"

namespace DOH {

	class KeyEvent : public AEvent {

	private:
		int mKeyCode;

	protected:
		KeyEvent(EEventType type, int keyCode)
		:	AEvent(type, EEventCategory::KEY),
			mKeyCode(keyCode)
		{}

	public:
		inline int getKeyCode() const { return mKeyCode; }
	};

	class KeyDownEvent : public KeyEvent {

	public:
		KeyDownEvent(int keyCode)
		:	KeyEvent(EEventType::KEY_DOWN, keyCode)
		{}
	};

	class KeyUpEvent : public KeyEvent {

	public:
		KeyUpEvent(int keyCode)
			: KeyEvent(EEventType::KEY_UP, keyCode)
		{}
	};
}
