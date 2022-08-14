#pragma once

#include "dough/Utils.h"
#include "dough/Window.h"
#include "dough/rendering/RendererVulkan.h"
#include "dough/events/WindowEvent.h"
#include "dough/events/KeyEvent.h"
#include "dough/events/MouseEvent.h"
#include "dough/input/Input.h"
#include "dough/application/IApplicationLogic.h"
#include "dough/application/ApplicationLoop.h"
#include "dough/time/IntervalTimer.h"
#include "dough/application/ApplicationInitSettings.h"

namespace DOH {

	struct AppDebugInfo {
		const static int FrameTimesCount = 1000;

		float FrameTimesMillis[FrameTimesCount] = {};
		
		double LastFrameTimeMillis = 0.0;
		double LastUpdateTimeMillis = 0.0;
		double LastRenderTimeMillis = 0.0;

		int FrameTimeIndex = 0;
		bool FrameTimesFullArray = false;

		uint32_t SceneDrawCalls = 0;
		uint32_t UiDrawCalls = 0;
		uint32_t BatchRendererDrawCalls = 0;

		uint32_t TotalDrawCalls = 0;

		inline void updateTotalDrawCallCount() {
			TotalDrawCalls = SceneDrawCalls + UiDrawCalls + BatchRendererDrawCalls;
		}

		inline void resetDrawCalls() {
			SceneDrawCalls = 0;
			UiDrawCalls = 0;
			BatchRendererDrawCalls = 0;
		}
	};

	class Application {

		friend class ApplicationLoop;

	private:
		static Application* INSTANCE;

		std::unique_ptr<Window> mWindow;
		std::unique_ptr<ApplicationLoop> mAppLoop;
		std::unique_ptr<RendererVulkan> mRenderer;
		std::shared_ptr<IApplicationLogic> mAppLogic;
		std::unique_ptr<IntervalTimer> mAppInfoTimer;
		std::unique_ptr<AppDebugInfo> mAppDebugInfo;
		bool mRunning;
		bool mFocused;
		bool mIconified;

	public:
		Application(const Application& copy) = delete;
		void operator=(const Application& assignment) = delete;
		static Application& get() { return *INSTANCE; }

		void run();
		void stop() { mRunning = false; }

		void onWindowEvent(WindowEvent& windowEvent);
		void onKeyEvent(KeyEvent& keyEvent);
		void onMouseEvent(MouseEvent& mouseEvent);

		inline RendererVulkan& getRenderer() const { return *mRenderer; }
		inline Window& getWindow() const { return *mWindow; }
		inline ApplicationLoop& getLoop() const { return *mAppLoop; }
		inline IntervalTimer& getAppInfoTimer() const { return *mAppInfoTimer; }
		inline AppDebugInfo& getDebugInfo() const { return *mAppDebugInfo; }
		inline bool isRunning() const { return mRunning; }
		inline bool isFocused() const { return mFocused; }
		inline bool isIconified() const { return mIconified; }

		static int start(std::shared_ptr<IApplicationLogic> appLogic, ApplicationInitSettings initSettings);
		static bool isInstantiated() { return INSTANCE != nullptr; };

	private:
		Application();

		void init(std::shared_ptr<IApplicationLogic> appLogic, const ApplicationInitSettings& initSettings);
		inline void pollEvents() const { mWindow->pollEvents(); }
		void update(float delta);
		void render(float delta);
		void close();
	};
}
