#include "dough/time/IntervalTimer.h"
#include "dough/Logging.h"

namespace DOH {

	IntervalTimer::IntervalTimer(const std::string& name, bool start)
	:	Timer(name, start),
		mRecordedIntervalsMillis({})
	{}

	void IntervalTimer::recordInterval(const std::string& label) {
		if (mTicking) {
			mRecordedIntervalsMillis.emplace(getCurrentTickingTimeMillis(), label);
		} else {
			LOGLN("Can't record interval when timer isn't ticking.");
		}
	}

	void IntervalTimer::reset() {
		mStartTimeMillis = 0.0;
		mEndTimeMillis = 0.0;
		mTicking = false;
		mRecordedIntervalsMillis.clear();
	}

	void IntervalTimer::dump() {
		LOGLN_UNDERLINED(
			"IntervalTimer dump: \"" << mName << "\" Total ticking time: " 
			<< Time::convertMillisToSeconds(getTotalTickingTimeMillis()) << "s"
		);
		if (hasIntervalsRecorded()) {
			LOGLN("Recorded intervals:");
			for (auto& [recordTime, label] : getRecordedIntervalsMillis()) {
				LOGLN("--" << label << " : " << Time::convertMillisToSeconds(recordTime) << "s");
			}
		}
	}
}
