#pragma once

#include "dough/time/Time.h"

#include <string>

namespace DOH {

	class Timer {

	protected:
		std::string mName;
		double mStartTimeMillis;
		double mEndTimeMillis;
		bool mTicking;

	public:
		Timer(const std::string& name, bool start = false);

		void start();
		void end();

		virtual void reset();
		virtual void dump();

		inline double getStartTimeMillis() const { return mStartTimeMillis; }
		inline double getEndTimeMillis() const { return mEndTimeMillis; }
		inline double getCurrentTickingTimeMillis() const { return (Time::getCurrentTimeMillis() - mStartTimeMillis); }
		inline double getTotalTickingTimeMillis() const { return mEndTimeMillis - mStartTimeMillis; }
		inline bool isTicking() const { return mTicking; }

	};

}