#pragma once

#define DEBUG_MEM_USE_DOH

#include "dough/Utils.h"
#include "dough/Window.h"
#include "dough/rendering/RendererVulkan.h"
#include "dough/events/WindowEvent.h"
#include "dough/events/KeyEvent.h"
#include "dough/events/MouseEvent.h"
#include "dough/input/Input.h"

namespace DOH {

	class Application {

	private:

		static Application* INSTANCE;

		std::unique_ptr<Window> mWindow;
		std::unique_ptr<RendererVulkan> mRenderer;
		bool mRunning;

	public:
		Application(const Application& copy) = delete;
		void operator=(const Application& assignment) = delete;

		void run();
		void stop() { mRunning = false; }

		void onWindowEvent(WindowEvent& windowEvent);
		void onKeyEvent(KeyEvent& keyEvent);
		void onMouseEvent(MouseEvent& mouseEvent);

		inline RendererVulkan& getRenderer() const { return *mRenderer; }
		inline Window& getWindow() const { return *mWindow; }

		static Application& get() { return *INSTANCE; }
		static int start();
		static bool isInstantiated() { return INSTANCE != nullptr; };

	private:
		Application();

		void init();
		void mainLoop();
		void update();
		void render();
		void close();
	};
}
