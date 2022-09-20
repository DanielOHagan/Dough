#include "dough/time/IntervalTimer.h"
#include "dough/Logging.h"

namespace DOH {

	IntervalTimer::IntervalTimer(bool start)
	:	Timer(start),
		mRecordedIntervalsMillis({})
	{}

	void IntervalTimer::recordInterval(const std::string& label) {
		if (mTicking) {
			mRecordedIntervalsMillis.emplace_back(getCurrentTickingTimeMillis(), label);
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

	void IntervalTimer::dump(const std::string& label) {
		LOGLN_UNDERLINED(
			"\"" << label << "\" Interval Timer dump: Total ticking time: "
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
