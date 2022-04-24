#include "dough/application/ApplicationLoop.h"
#include "dough/Logging.h"

#include <algorithm>

namespace DOH {

	const float ApplicationLoop::MAX_TARGET_FPS = 360.0f;
	const float ApplicationLoop::MIN_TARGET_FPS = 15.0f;
	const float ApplicationLoop::MAX_TARGET_UPS = 1000.0f;
	const float ApplicationLoop::MIN_TARGET_UPS = 15.0f;
	const float ApplicationLoop::DEFAULT_TARGET_FPS = 120.0f;
	const float ApplicationLoop::DEFAULT_TARGET_BACKGROUND_FPS = 15.0f;
	const float ApplicationLoop::DEFAULT_TARGET_UPS = 144.0f;
	const float ApplicationLoop::DEFAULT_TARGET_BACKGROUND_UPS = 15.0f;
	const bool ApplicationLoop::DEFAULT_RUN_IN_BACKGROUND = false;


	ApplicationLoop::ApplicationLoop(
		Application& app,
		float targetFps,
		float targetUps
	) : mApplication(app),
		mTargetFrameTimeSpan(0),
		mLastCycleTimePoint(Time::getCurrentTimeMillis()),
		mPerSecondCountersTimeSpan(0.0f),
		mDeltaRenderTimeSpan(0.0),
		mLastRenderTimePoint(0.0),
		mTargetUpdateTimeSpan(0),
		mDeltaUpdateTimeSpan(0.0),
		mLastUpdateTimePoint(0.0),
		mFps(0.0f),
		mTargetFps(targetFps),
		mTargetBackgroundFps(DEFAULT_TARGET_BACKGROUND_FPS),
		mUps(0.0f),
		mTargetUps(targetUps),
		mTargetBackgroundUps(DEFAULT_TARGET_BACKGROUND_UPS),
		mPreviousFps(0.0f),
		mPreviousUps(0.0f),
		mRunInBackground(false)
	{
		updateTargetUpdateTime(app.isFocused());
		updateTargetFrameTime(app.isFocused());
	}

	ApplicationLoop::ApplicationLoop(
		Application& app,
		float targetFps,
		float targetUps,
		bool runInBackground,
		float targetBackgroundFps,
		float targetBackgroundUps
	) : mApplication(app),
		mTargetFrameTimeSpan(0),
		mLastCycleTimePoint(Time::getCurrentTimeMillis()),
		mPerSecondCountersTimeSpan(0.0f),
		mDeltaRenderTimeSpan(0.0),
		mLastRenderTimePoint(0.0),
		mTargetUpdateTimeSpan(0),
		mDeltaUpdateTimeSpan(0.0),
		mLastUpdateTimePoint(0.0),
		mFps(0.0f),
		mTargetFps(targetFps),
		mTargetBackgroundFps(targetBackgroundFps),
		mUps(0.0f),
		mTargetUps(targetUps),
		mTargetBackgroundUps(targetBackgroundUps),
		mPreviousFps(0.0f),
		mPreviousUps(0.0f),
		mRunInBackground(runInBackground)
	{
		updateTargetFrameTime(app.isFocused());
		updateTargetUpdateTime(app.isFocused());
	}

	void ApplicationLoop::run() {
		while (mApplication.isRunning()) {
			mApplication.pollEvents();

			const double deltaCycleTimeSpan = Time::getCurrentTimeMillis() - mLastCycleTimePoint;

			mDeltaUpdateTimeSpan += deltaCycleTimeSpan;
			mDeltaRenderTimeSpan += deltaCycleTimeSpan;
			mPerSecondCountersTimeSpan += deltaCycleTimeSpan;

			if (mPerSecondCountersTimeSpan > 1000.0) {
				mPreviousFps = mFps;
				mFps = 0.0f;
				mPreviousUps = mUps;
				mUps = 0.0f;

				const bool noLimitFps = mApplication.isFocused() || mRunInBackground;
				LOG(
					"FPS: " << mPreviousFps << " (" <<
					(noLimitFps ? mTargetFps : mTargetBackgroundFps) << ")"
				);
				LOGLN(
					"\tUPS: " << mPreviousUps << " (" <<
					(noLimitFps ? mTargetUps : mTargetBackgroundUps) << ")"
				);

				mPerSecondCountersTimeSpan = 0.0;
			}
			
			if (!(mDeltaUpdateTimeSpan < mTargetUpdateTimeSpan)) {
				mApplication.update(Time::convertMillisToSeconds(mDeltaUpdateTimeSpan));
				mUps++;
				mDeltaUpdateTimeSpan = 0.0;
				mLastUpdateTimePoint = Time::getCurrentTimeMillis();
			}

			if (!(mDeltaRenderTimeSpan < mTargetFrameTimeSpan)) {
				//NOTE:: When Iconified the app doesn't call any render functions even though
				//	it is called here and the FPS increments here
				if (!mApplication.isIconified()) {
					mApplication.render();
					mFps++;
					mDeltaRenderTimeSpan = 0.0;
					mLastRenderTimePoint = Time::getCurrentTimeMillis();
				}
			}

			mLastCycleTimePoint = Time::getCurrentTimeMillis();
		}
	}

	void ApplicationLoop::setTargetFps(float targetFps, bool includeTargetBackgroundFps) {
		mTargetFps = std::clamp(targetFps, MIN_TARGET_FPS, std::min(mTargetUps, MAX_TARGET_FPS));

		if (includeTargetBackgroundFps) {
			mTargetBackgroundFps = mTargetFps;
		}

		updateTargetFrameTime(mApplication.isFocused());
	}

	void ApplicationLoop::setTargetBackgroundFps(float targetBackgroundFps) {
		mTargetBackgroundFps = std::clamp(targetBackgroundFps, MIN_TARGET_FPS, MAX_TARGET_FPS);

		updateTargetFrameTime(mApplication.isFocused());
	}

	void ApplicationLoop::setTargetUps(float targetUps, bool includeTargetBackgroundUps) {
		mTargetUps = std::clamp(targetUps, std::max(mTargetFps, MIN_TARGET_UPS), MAX_TARGET_UPS);

		if (includeTargetBackgroundUps) {
			mTargetBackgroundUps = mTargetUps;
		}

		updateTargetUpdateTime(mApplication.isFocused());
	}

	void ApplicationLoop::setTargetBackgroundUps(float targetBackgroundUps) {
		mTargetBackgroundUps = std::clamp(targetBackgroundUps, std::max(mTargetBackgroundUps, MIN_TARGET_UPS), MAX_TARGET_UPS);

		updateTargetUpdateTime(mApplication.isFocused());
	}
}
