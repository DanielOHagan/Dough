#include "dough/time/PausableTimer.h"

#include "dough/Logging.h"

namespace DOH {

	PausableTimer::PausableTimer(bool start)
	:	Timer(start),
		mLastPauseTimePoint(0.0),
		mPaused(!start)
	{
		if (start) {
			this->start();
		}
	}

	void PausableTimer::start() {
		if (!mTicking) {
			mStartTimeMillis = Time::getCurrentTimeMillis();
			mTicking = true;
			mPaused = false;
		} else {
			LOGLN("Can't start an already ticking timer.");
		}
	}

	void PausableTimer::end() {
		if (mTicking) {
			mEndTimeMillis = mPaused ? mLastPauseTimePoint : Time::getCurrentTimeMillis();
			mTicking = false;
			mPaused = true;
		} else {
			LOGLN("Can't stop a clock that isn't ticking.")
		}
	}

	void PausableTimer::reset() {
		Timer::reset();

		mLastPauseTimePoint = 0.0;
		mPaused = true;
	}

	double PausableTimer::getCurrentTickingTimeMillis() const {
		return mTicking ?
			mPaused ?
				mLastPauseTimePoint - mStartTimeMillis
				: (Time::getCurrentTimeMillis() - mStartTimeMillis)
			: getTotalTickingTimeMillis();
	}

	void PausableTimer::pause() {
		if (!mTicking && mPaused) {
			LOG_WARN("Timer must be ticking and not already paused");
			return;
		}

		mLastPauseTimePoint = Time::getCurrentTimeMillis();
		mPaused = true;
	}

	void PausableTimer::unPause() {
		if (!mTicking && !mPaused) {
			LOG_WARN("Timer must ticking and already paused");
			return;
		}

		//Take time paused and add it to original start time
		mStartTimeMillis += Time::getCurrentTimeMillis() - mLastPauseTimePoint;
		mLastPauseTimePoint = 0.0;
		mPaused = false;
	}
}
