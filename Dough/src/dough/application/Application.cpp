#include "dough/application/Application.h"

namespace DOH {

	Application* Application::INSTANCE = nullptr;

	Application::Application()
	:	mRunning(false)
	{
		
	}

	Application::~Application() {
		if (Application::isInstantiated()) {
			delete this;
		}
	}

	void Application::run(/*IAppLogic appLogic*/) {
		init(/*appLogic*/);

		mainLoop();

		close();
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
	}

	int Application::start(/*IAppLogic appLogic*/) {
		INSTANCE = new Application();
		
		if (Application::isInstantiated()) {
			try {
				INSTANCE->run();
				return EXIT_SUCCESS;
			} catch (const std::exception& e) {
				std::cerr << e.what() << std::endl;
				return EXIT_FAILURE;
			}
		} else {
			return EXIT_FAILURE;
		}
	}
}
