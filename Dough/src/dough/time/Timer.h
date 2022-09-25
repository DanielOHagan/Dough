#pragma once

#include "dough/time/Time.h"

#include <string>

namespace DOH {

	class Timer {

	protected:
		double mStartTimeMillis;
		double mEndTimeMillis;
		bool mTicking;

	public:
		Timer(bool start = false);

		virtual void start();
		virtual void end();
		virtual void reset();
		virtual void dump(const std::string& label);
		virtual double getCurrentTickingTimeMillis() const;

		inline double getStartTimeMillis() const { return mStartTimeMillis; }
		inline double getEndTimeMillis() const { return mEndTimeMillis; }
		inline double getTotalTickingTimeMillis() const { return mEndTimeMillis - mStartTimeMillis; }
		inline bool isTicking() const { return mTicking; }
	};
}