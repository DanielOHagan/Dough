#pragma once

#include "dough/time/Timer.h"

#include <vector>
#include <string>

namespace DOH {
	
	class IntervalTimer : public Timer {

	private:
		std::vector<std::pair<double, const std::string>> mRecordedIntervalsMillis;

	public:
		IntervalTimer(bool start = false);

		void recordInterval(const std::string& label);
		virtual void reset() override;
		virtual void dump(const std::string& label) override;

		inline bool hasIntervalsRecorded() const { return mRecordedIntervalsMillis.size() > 0; }
		inline const std::vector<std::pair<double, const std::string>>& getRecordedIntervalsMillis() const { return mRecordedIntervalsMillis; }
	};
}
