#include "dough/time/Timer.h"
#include "dough/Logging.h"

namespace DOH {
	
	Timer::Timer(bool start)
	:	mStartTimeMillis(start ? Time::getCurrentTimeMillis() : 0.0),
		mEndTimeMillis(0.0),
		mTicking(start)
	{}

	void Timer::start() {
		if (!mTicking) {
			mStartTimeMillis = Time::getCurrentTimeMillis();
			mTicking = true;
		} else {
			LOGLN("Can't start an already ticking timer.");
		}
	}

	void Timer::end() {
		if (mTicking) {
			mEndTimeMillis = Time::getCurrentTimeMillis();
			mTicking = false;
		} else {
			LOGLN("Can't stop a clock that isn't ticking.")
		}
	}

	void Timer::reset() {
		mStartTimeMillis = 0.0;
		mEndTimeMillis = 0.0;
		mTicking = false;
	}

	void Timer::dump(const std::string& label) {
		LOGLN("\"" << label << "\" Total ticking time: " << Time::convertMillisToSeconds(getTotalTickingTimeMillis()) << "s");
	}
}
