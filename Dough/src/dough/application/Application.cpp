#include "dough/application/Application.h"

namespace DOH {

	Application* Application::INSTANCE = nullptr;

	//TEMP::
	uint32_t Application::resizeCount = 0;

	Application::Application()
	:	mRunning(false)
	{
		
	}

	void Application::run(/*IAppLogic appLogic*/) {
		init(/*appLogic*/);

		mainLoop();
	}

	void Application::init() {
		mWindow = std::make_unique<Window>(800, 600);
		mWindow->init();

		mRenderer = std::make_unique<RendererVulkan>();
		mRenderer->init(mWindow->getWidth(), mWindow->getHeight());
	}

	void Application::mainLoop() {
		mRunning = true;

		while (mRunning) {
			mWindow->pollEvents();
			update();
			render();
		}
	}

	void Application::update() {
		//mAppLogic->update(delta);
	}

	void Application::render() {
		mRenderer->drawFrame();
	}

	void Application::close() {
		mWindow->close();
		mRenderer->close();

		//TEMP::
		std::cout << "rc: " << resizeCount << std::endl;

		delete INSTANCE;
	}

	int Application::start(/*IAppLogic appLogic*/) {
		INSTANCE = new Application();
		
		if (Application::isInstantiated()) {
			try {
				INSTANCE->run();
				Application::get().close();
				return EXIT_SUCCESS;
			} catch (const std::exception& e) {
				Application::get().close();
				std::cerr << e.what() << std::endl;
				return EXIT_FAILURE;
			}
		} else {
			return EXIT_FAILURE;
		}
	}
}
