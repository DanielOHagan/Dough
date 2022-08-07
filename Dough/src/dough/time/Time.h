#pragma once

#include <chrono>

namespace DOH {

	//Alias for brevity and use of same type of clock
	using TimePoint = std::chrono::high_resolution_clock::time_point;
	using TimeSpan = std::chrono::high_resolution_clock::duration;

	class Time {

	private:
		Time() = delete;
		Time(const Time& copy) = delete;
		Time operator=(const Time& assignment) = delete;

	public:

		static long long getCurrentTimeNanos() { return std::chrono::high_resolution_clock::now().time_since_epoch().count(); }
		static long getCurrentTimeMicro() { return (long) getCurrentTimeNanos() / (long) 1000.0; }
		static double getCurrentTimeMillis() { return (double) getCurrentTimeNanos() / 1000000.0; }
		static float getCurrentTimeSeconds() { return (float) (getCurrentTimeNanos() / 1000000000.0); }

		static float convertNanosToMinutes(long long nanos) { return (float) (nanos / (1000000000ll * 60)); }
		static float convertMicrosToMinutes(long micros) { return (float) (micros / (1000000 * 60)); }
		static float convertMillisToMinutes(double millis) { return (float) (millis / (1000 * 60)); }
		static float convertSecondsToMinutes(float seconds) { return seconds / 60.0f; }

		static float convertNanosToSeconds(long long nanos) { return (float) nanos / 1000000000; }
		static float convertMicrosToSeconds(long micros) { return (float) micros / 1000000; }
		static float convertMillisToSeconds(double millis) { return (float) millis / 1000; }

		static double convertNanosToMillis(long long nanos) { return (double) nanos / 1000000; }
		static double convertMicrosToMillis(long micros) { return (double) micros / 1000; }
		static double convertSecondsToMillis(float seconds) { return (double) seconds * 1000; }

		static long convertNanosToMicros(long long nanos) { return (long) nanos / 1000; }
		static long convertMillisToMicros(double millis) { return (long) millis * 1000; }
		static long convertSecondsToMicros(float seconds) { return (long) seconds * 1000000; }

		static long long convertMicrosToNanos(long micros) { return (long long) micros * 1000; }
		static long long convertMillisToNanos(double millis) { return (long long) millis * 1000000; }
		static long long convertSecondsToNanos(float seconds) { return (long long) seconds * 1000000000; }

	};

}
