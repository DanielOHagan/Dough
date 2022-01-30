#pragma once

#include "dough/application/Application.h"
#include "dough/time/Time.h"

namespace DOH {

	//TODO:: Having a higher FPS causes the display to "flash", maybe I should force the UPS to always be equal-to/higher than FPS

	class ApplicationLoop {

	public:
		
		static const float MAX_TARGET_FPS;
		static const float MIN_TARGET_FPS;
		static const float MAX_TARGET_UPS;
		static const float MIN_TARGET_UPS;
		static const float DEFAULT_TARGET_FPS;
		static const float DEFAULT_TARGET_BACKGROUND_FPS;
		static const float DEFAULT_TARGET_UPS;
		static const float DEFAULT_TARGET_BACKGROUND_UPS;
		static const bool DEFAULT_RUN_IN_BACKGROUND;

	private:
		Application& mApplication;

		double mTargetFrameTimeSpan;
		double mLastCycleTimePoint;
		double mPerSecondCountersTimeSpan;
		double mDeltaRenderTimeSpan;
		double mLastRenderTimePoint;
		double mTargetUpdateTimeSpan;
		double mDeltaUpdateTimeSpan;
		double mLastUpdateTimePoint;

		float mFps;
		float mTargetFps;
		float mTargetBackgroundFps;
		float mUps;
		float mTargetUps;
		float mTargetBackgroundUps;
		float mPreviousFps;
		float mPreviousUps;
		
		bool mRunInBackground;

	public:
		ApplicationLoop(
			Application& app,
			float targetFps = DEFAULT_TARGET_FPS,
			bool runInBackground = DEFAULT_RUN_IN_BACKGROUND,
			float targetBackgroundFps = DEFAULT_TARGET_BACKGROUND_FPS
		);
		ApplicationLoop(const ApplicationLoop& copy) = delete;
		ApplicationLoop operator=(const ApplicationLoop& assignment) = delete;

		void run();

		inline void onFocusChange(bool focused) { updateTargetFrameTime(focused); updateTargetUpdateTime(focused); }
		inline void updateTargetFrameTime(bool focused) { mTargetFrameTimeSpan = 1000.0 / (focused ? mTargetFps : mTargetBackgroundFps); }
		inline void updateTargetUpdateTime(bool focused) { mTargetUpdateTimeSpan = 1000.0 / (focused ? mTargetUps : mTargetBackgroundUps); }

		inline bool isRunningInBackground() const { return mRunInBackground; }
		inline void setRunInBackground(bool renerInBackground) { mRunInBackground = renerInBackground; }
		
		inline float getTargetFps() const { return mTargetFps; }
		void setTargetFps(float targetFps, bool includeTargetBackgroundFps = false);
		inline float getTargetBackgroundFps() const { return mTargetBackgroundFps; }
		void setTargetBackgroundFps(float targetBackgroundFps);
		inline float getFps() const { return mPreviousFps; } //Number of renders in last second-long interval
		
		inline float getTargetUps() const { return mTargetUps; }
		void setTargetUps(float targetUps, bool includeTargetBackroundUps = false);
		inline float getTargetBackgroundUps() const { return mTargetBackgroundUps; }
		void setTargetBackgroundUps(float targetBackgroundUps);
		inline float getUps() const { return mPreviousUps; } //Number of updates in last second-long interval
	};
}
