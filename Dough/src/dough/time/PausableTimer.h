#pragma once

#include "dough/time/Timer.h"

namespace DOH {

	class PausableTimer : public Timer {

	private:
		double mLastPauseTimePoint;
		bool mPaused;

	public:
		PausableTimer(bool start = false);

		virtual void start() override;
		virtual void end() override;
		virtual void reset() override;
		virtual double getCurrentTickingTimeMillis() const override;

		void pause();
		void unPause();

		inline bool isPaused() const { return mPaused; }
	};
}
