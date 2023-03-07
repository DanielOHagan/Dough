#pragma once

#include <chrono>
#include <array>

namespace DOH {

	//Alias for brevity and use of same type of clock
	using TimePoint = std::chrono::high_resolution_clock::time_point;
	using TimeSpan = std::chrono::high_resolution_clock::duration;

	static std::array<const char*, 6> ETimeUnitStrings = {
		"HOUR",
		"MINUTE",
		"SECOND",
		"MILLISECOND",
		"MICROSECOND",
		"NANOSECOND"
	};

	static std::array<const char*, 6> ETimeUnitStringsShorthand = {
		"hr",
		"min",
		"s",
		"ms",
		"micro", //"µm", Default ImGui font doesn't contain the 'µ' | TODO:: using "micro" as default until editor fonts are available
		"ns"
	};

	enum ETimeUnit {
		HOUR,
		MINUTE,
		SECOND,
		MILLISECOND,
		MICROSECOND,
		NANOSECOND
	};

	class Time {

	private:
		Time() = delete;
		Time(const Time& copy) = delete;
		Time operator=(const Time& assignment) = delete;

	public:

		static const uint64_t getCurrentTimeNanos() { return std::chrono::high_resolution_clock::now().time_since_epoch().count(); }
		static const long getCurrentTimeMicro() { return static_cast<long>(getCurrentTimeNanos() / 1000ll); }
		static const double getCurrentTimeMillis() { return static_cast<double>(getCurrentTimeNanos() / 1000000.0); }
		static const float getCurrentTimeSeconds() { return static_cast<float>(getCurrentTimeNanos() / 1000000000.0); }

		static const float convertNanosToMinutes(long long nanos) { return static_cast<float>(nanos / (1000000000ll * 60)); }
		static const float convertMicrosToMinutes(long micros) { return static_cast<float>(micros / (1000000 * 60)); }
		static const float convertMillisToMinutes(double millis) { return static_cast<float>(millis / (1000 * 60)); }
		static const float convertSecondsToMinutes(float seconds) { return seconds / 60.0f; }

		static const float convertNanosToSeconds(long long nanos) { return static_cast<float>(nanos / 1000000000); }
		static const float convertMicrosToSeconds(long micros) { return static_cast<float>(micros / 1000000); }
		static const float convertMillisToSeconds(double millis) { return static_cast<float>(millis / 1000); }

		static const double convertNanosToMillis(long long nanos) { return static_cast<double>(nanos / 1000000); }
		static const double convertMicrosToMillis(long micros) { return static_cast<double>(micros / 1000); }
		static const double convertSecondsToMillis(float seconds) { return static_cast<double>(seconds * 1000); }

		static const long convertNanosToMicros(long long nanos) { return static_cast<long>(nanos / 1000); }
		static const long convertMillisToMicros(double millis) { return static_cast<long>(millis * 1000); }
		static const long convertSecondsToMicros(float seconds) { return static_cast<long>(seconds * 1000000); }

		static const uint64_t convertMicrosToNanos(long micros) { return static_cast<long long>(micros * 1000); }
		static const uint64_t convertMillisToNanos(double millis) { return static_cast<long long>(millis * 1000000); }
		static const uint64_t convertSecondsToNanos(float seconds) { return static_cast<long long>(seconds * 1000000000); }
	};
}
