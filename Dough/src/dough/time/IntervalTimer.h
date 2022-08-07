#pragma once

#include "dough/time/Timer.h"

#include <unordered_map>
#include <string>

namespace DOH {
	
	class IntervalTimer : public Timer {

	private:
		std::unordered_map<double, const std::string> mRecordedIntervalsMillis;

	public:
		IntervalTimer(const std::string& name, bool start = false);

		void recordInterval(const std::string& label);
		virtual void reset() override;
		virtual void dump() override;

		inline bool hasIntervalsRecorded() const { return mRecordedIntervalsMillis.size() > 0; }
		inline std::unordered_map<double, const std::string> getRecordedIntervalsMillis() const { return mRecordedIntervalsMillis; }

	};
}
