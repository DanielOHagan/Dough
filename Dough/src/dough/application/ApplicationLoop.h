#pragma once

#include "dough/application/Application.h"
#include "dough/time/Time.h"

namespace DOH {

	class ApplicationLoop {

	public:
		
		static const float MAX_TARGET_FPS;
		static const float MIN_TARGET_FPS;
		static const float MAX_TARGET_UPS;
		static const float MIN_TARGET_UPS;

	private:
		Application& mApplication;

		double mTargetFrameTimeSpan;
		double mLastCycleTimePoint;
		double mPerSecondCountersTimeSpan;
		double mDeltaRenderTimeSpan;
		double mTargetUpdateTimeSpan;
		double mDeltaUpdateTimeSpan;

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
			float targetFps,
			float targetUps,
			bool runInBackground,
			float targetBackgroundFps,
			float targetBackgroundUps
		);
		ApplicationLoop(const ApplicationLoop& copy) = delete;
		ApplicationLoop operator=(const ApplicationLoop& assignment) = delete;

		void run();

		inline void onFocusChange(bool focused) { updateTargetFrameTime(focused); updateTargetUpdateTime(focused); }
		inline void updateTargetFrameTime(bool focused) {
			const float targetFpsTimeSpan = focused ? mTargetFps : mRunInBackground ? mTargetFps : mTargetBackgroundFps;
			mTargetFrameTimeSpan = 1000.0 / targetFpsTimeSpan;
		}
		inline void updateTargetUpdateTime(bool focused) {
			const float targetUpsTimeSpan = focused ? mTargetUps : mRunInBackground ? mTargetUps : mTargetBackgroundUps;
			mTargetUpdateTimeSpan = 1000.0 / targetUpsTimeSpan;
		}

		inline bool isRunningInBackground() const { return mRunInBackground; }
		inline void setRunInBackground(bool renderInBackground) { mRunInBackground = renderInBackground; }
		
		inline float getTargetFps() const { return mTargetFps; }
		void setTargetFps(float targetFps, bool includeTargetBackgroundFps = false);
		inline float getTargetBackgroundFps() const { return mTargetBackgroundFps; }
		void setTargetBackgroundFps(float targetBackgroundFps);
		inline float getFps() const { return mPreviousFps; } //Number of renders in last second-long interval
		inline bool isValidTargetFps(const float targetFps) const {
			if (targetFps > MAX_TARGET_FPS) return false;
			if (targetFps < MIN_TARGET_FPS) return false;
			if (targetFps > mTargetUps) return false;

			return true;
		}
		inline bool isValidTargetUps(const float targetUps) const {
			if (targetUps > MAX_TARGET_UPS) return false;
			if (targetUps < MIN_TARGET_UPS) return false;
			if (targetUps < mTargetFps) return false;

			return true;
		}

		inline float getTargetUps() const { return mTargetUps; }
		void setTargetUps(float targetUps, bool includeTargetBackroundUps = false);
		inline float getTargetBackgroundUps() const { return mTargetBackgroundUps; }
		void setTargetBackgroundUps(float targetBackgroundUps);
		inline float getUps() const { return mPreviousUps; } //Number of updates in last second-long interval

		inline double getTargetFrameTime() const { return mTargetFrameTimeSpan; }
		inline double getTargetUpdateTime() const { return mTargetUpdateTimeSpan; }
	};
}
