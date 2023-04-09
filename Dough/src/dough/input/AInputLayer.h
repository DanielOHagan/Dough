#pragma once

#include <glm/vec2.hpp>

namespace DOH {

	class AInputLayer {
	protected:
		const char* mName;
		bool mEnabled;

		AInputLayer(const char* name)
		:	mName(name),
			mEnabled(true)
		{}

	public:
		virtual bool handleKeyPressed(int keyCode, bool pressed) = 0;
		virtual bool handleMouseButtonPressed(int button, bool pressed) = 0;
		virtual bool handleMouseMoved(float x, float y) = 0;
		virtual bool handleMouseScroll(float offsetX, float offsetY) = 0;
		virtual void resetCycleData() = 0;
		virtual void reset() = 0;

		virtual inline bool hasDeviceInput() const { return false; }

		virtual bool isKeyPressed(int keyCode) const = 0;
		virtual bool isMouseButtonPressed(int button) const = 0;
		virtual inline bool isMouseScrollingUp() const = 0;
		virtual inline bool isMouseScrollingDown() const = 0;
		virtual inline const glm::vec2 getCursorPos() const = 0;

		inline void setEnabled(bool enabled) { mEnabled = enabled; }
		inline const bool isEnabled() const { return mEnabled; }
		inline const char* getName() const { return mName; }
	};
}
