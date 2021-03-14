#pragma once

#include "dough/Window.h"

namespace DOH {

	class Application {

	private:

		Window mWindow;

	public:
		Application()
			: mWindow(800, 600) {
		}

		void run() {
			init();

			mainLoop();

			close();
		}

	private:
		void init() {
			mWindow.init();
		}

		void mainLoop() {
			while (!mWindow.shouldClose()) {
				mWindow.pollEvents();
				update();
			}
		}

		void update() {
			mWindow.drawFrame();
		}

		void close() {
			mWindow.close();
		}
	};
}