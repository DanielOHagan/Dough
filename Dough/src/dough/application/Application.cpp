#include "dough/application/Application.h"

namespace DOH {

	Application::Application()
		: mWindow(800, 600) 		{
	}

	void Application::run() {
		init();

		mainLoop();

		close();
	}

	void Application::init() {
		mWindow.init();
	}

	void Application::mainLoop() {
		while (!mWindow.shouldClose()) {
			mWindow.pollEvents();
			update();
			render();
		}
	}

	void Application::update() {
		
	}

	void Application::render() {
		mWindow.drawFrame();
	}

	void Application::close() {
		mWindow.close();
	}
}