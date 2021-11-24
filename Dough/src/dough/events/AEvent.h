#pragma once

#include "dough/events/EEventType.h"
#include "dough/events/EEventCategory.h"

namespace DOH {

	class AEvent {

	protected:
		EEventType mType;
		EEventCategory mCategory;

	public:
		AEvent(EEventType type, EEventCategory category)
		:	mType(type),
			mCategory(category)
		{};
		AEvent(const AEvent& copy) = delete;
		AEvent operator=(const AEvent& assignment) = delete;

		inline EEventType getType() const { return mType; }
		inline EEventCategory getCategory() const { return mCategory; }

		static EEventCategory getCategoryFromType(EEventType type) {
			switch (type) {
				//Application


				case EEventType::WINDOW_RESIZE:
				case EEventType::WINDOW_CLOSE:
				case EEventType::WINDOW_FOCUS_CHANGE:
					return EEventCategory::WINDOW;

				case EEventType::KEY_DOWN:
				case EEventType::KEY_UP:
					return EEventCategory::KEY;

				case EEventType::MOUSE_BUTTON_DOWN:
				case EEventType::MOUSE_BUTTON_UP:
				case EEventType::MOUSE_SCROLL:
				case EEventType::MOUSE_MOVE:
					return EEventCategory::MOUSE;


				case EEventType::NONE:
				default:
					return EEventCategory::NONE;
			}
		}
	};
}
