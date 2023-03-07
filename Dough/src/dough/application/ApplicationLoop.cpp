#include "dough/application/ApplicationLoop.h"
#include "dough/Logging.h"

#include <algorithm>

namespace DOH {

	ApplicationLoop::ApplicationLoop(
		Application& app,
		float targetFps,
		float targetUps,
		bool runInBackground,
		float targetBackgroundFps,
		float targetBackgroundUps
	) : mApplication(app),
		mCurrentFrame(0),
		mTargetFrameTimeSpan(0),
		mLastCycleTimePoint(Time::getCurrentTimeMillis()),
		mPerSecondCountersTimeSpan(0.0f),
		mDeltaRenderTimeSpan(0.0),
		mTargetUpdateTimeSpan(0),
		mDeltaUpdateTimeSpan(0.0),
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
		AppDebugInfo& debugInfo = mApplication.getDebugInfo();

		while (mApplication.isRunning()) {
			mApplication.pollEvents();

			const double currentTimePoint = Time::getCurrentTimeMillis();
			const double deltaCycleTimeSpan = currentTimePoint - mLastCycleTimePoint;

			mDeltaUpdateTimeSpan += deltaCycleTimeSpan;
			mDeltaRenderTimeSpan += deltaCycleTimeSpan;
			mPerSecondCountersTimeSpan += deltaCycleTimeSpan;

			if (mPerSecondCountersTimeSpan > 1000.0) {
				mPreviousFps = mFps;
				mFps = 0.0f;
				mPreviousUps = mUps;
				mUps = 0.0f;

				////Log FPS and UPS each second
				//const bool noLimitFps = mApplication.isFocused() || mRunInBackground;
				//LOG(
				//	"FPS: " << mPreviousFps << " (" <<
				//	(noLimitFps ? mTargetFps : mTargetBackgroundFps) << ")"
				//);
				//LOGLN(
				//	"\tUPS: " << mPreviousUps << " (" <<
				//	(noLimitFps ? mTargetUps : mTargetBackgroundUps) << ")"
				//);

				//Add last second's fps count to debug array
				if (debugInfo.FpsCountIndex == AppDebugInfo::FpsCount) {
					debugInfo.FpsCountIndex = 0;
					debugInfo.FpsCountArrayIsFull = true;
				}
				debugInfo.FpsArray[debugInfo.FpsCountIndex] = mPreviousFps;
				debugInfo.FpsCountIndex++;

				mPerSecondCountersTimeSpan = 0.0;
			}

			if (!(mDeltaUpdateTimeSpan < mTargetUpdateTimeSpan)) {
				mApplication.update(Time::convertMillisToSeconds(mDeltaUpdateTimeSpan));
				mUps++;
				mDeltaUpdateTimeSpan = 0.0;
			}

			if (!(mDeltaRenderTimeSpan < mTargetFrameTimeSpan)) {
				//NOTE:: When Iconified the app doesn't call any render functions even though
				//	it is called here and the FPS increments here

				const float deltaRender = Time::convertMillisToSeconds(mDeltaRenderTimeSpan);
				if (!mApplication.isIconified()) {
					mApplication.render(deltaRender);
					mCurrentFrame++;
					mFps++;
					mDeltaRenderTimeSpan = 0.0;
				}

				//IMPORTANT:: Update and render calls are not gauranteed to be 1-to-1 and there can be more update calls per frame than render calls.
				//	Only update the render time on render call
				if (debugInfo.FrameTimeIndex == AppDebugInfo::FrameTimesCount) {
					debugInfo.FrameTimeIndex = 0;
					debugInfo.FrameTimesArrayIsFull = true;
				}
				debugInfo.FrameTimesMillis[debugInfo.FrameTimeIndex] = static_cast<float>(debugInfo.LastUpdateTimeMillis + debugInfo.LastRenderTimeMillis);
				debugInfo.FrameTimeIndex++;
			}

			mLastCycleTimePoint = currentTimePoint;
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
