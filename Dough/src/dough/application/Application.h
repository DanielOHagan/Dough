#pragma once

#include "dough/Window.h"
#include "dough/rendering/RendererVulkan.h"

namespace DOH {

	class Application {

	private:

		Window mWindow;
		//RendererVulkan mRenderer;

	public:
		Application();

		void run();

	private:
		void init();
		void mainLoop();
		void update();
		void render();
		void close();
	};
}