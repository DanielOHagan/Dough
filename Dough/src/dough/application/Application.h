#pragma once

#define DEBUG_MEM_USE_DOH

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

namespace DOH {

	class Application {

		friend class ApplicationLoop;

	private:
		static Application* INSTANCE;

		std::unique_ptr<Window> mWindow;
		std::unique_ptr<ApplicationLoop> mAppLoop;
		std::unique_ptr<RendererVulkan> mRenderer;
		std::shared_ptr<IApplicationLogic> mAppLogic;
		std::unique_ptr<IntervalTimer> mAppInfoTimer;
		bool mRunning;
		bool mFocused;

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
		inline bool isRunning() const { return mRunning; }
		inline bool isFocused() const { return mFocused; }

		static int start(std::shared_ptr<IApplicationLogic> appLogic);
		static bool isInstantiated() { return INSTANCE != nullptr; };

	private:
		Application();

		void init(std::shared_ptr<IApplicationLogic> appLogic);
		inline void pollEvents() const { mWindow->pollEvents(); }
		void update(float delta);
		void render();
		void close();
	};
}
